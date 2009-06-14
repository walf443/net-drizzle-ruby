#include <ruby.h>
#ifdef RUBY_RUBY_H
    #include <ruby/encoding.h>
#endif

#include "drizzle.h"

VALUE rb_utf8_str_new(const char *str, long len)
{
#ifdef RUBY_ENCODING_H
    return rb_enc_str_new(str, len, rb_enc_find("UTF-8"));
#else
    return rb_str_new(str, len);
#endif
}

static void rb_drizzle_free(net_drizzle_st *context)
{
    drizzle_free(context->drizzle);
    context->drizzle = NULL;
    ruby_xfree(context);
}

static VALUE rb_drizzle_alloc(VALUE klass) 
{
    drizzle_st *drizzle;
    net_drizzle_st* context;
    VALUE self;

    if ( ( drizzle = drizzle_create(NULL) ) == NULL ) {
        rb_sys_fail("Failed to initialize instance of Net::Drizzle");
    }

    self = Data_Make_Struct(klass, net_drizzle_st, 0, rb_drizzle_free, context);
    context->drizzle = drizzle;

    return self;
}

VALUE rb_drizzle_con_create(VALUE self)
{
    VALUE mNet = rb_const_get(rb_cObject, rb_intern("Net")); 
    VALUE cDrizzle = rb_const_get(mNet, rb_intern("Drizzle")); 
    VALUE cConnection = rb_const_get(cDrizzle, rb_intern("Connection"));

    return rb_funcall(cConnection, rb_intern("new"), 1, self);
}

VALUE rb_drizzle_query_run_all(VALUE self)
{
    net_drizzle_st* context;
    Data_Get_Struct(self, net_drizzle_st, context);

    int ret = drizzle_query_run_all(context->drizzle);

    return INT2FIX(ret);
}

static void rb_drizzle_con_free(net_drizzle_con_st *context)
{
    if ( context->con != NULL ) {
        // conn may be free by drizzle_free.
        if ( context->con->drizzle != NULL ) {
            // FIXME: drizzle_free also free connection memory. So rely it.
            // drizzle_con_free(context->con);
        }
        context->con = NULL;
    }

    ruby_xfree(context);
}

static VALUE rb_drizzle_con_alloc(VALUE klass)
{
    VALUE self;
    net_drizzle_con_st *context;
    self = Data_Make_Struct(klass, net_drizzle_con_st, 0, rb_drizzle_con_free, context);

    return self;
}

VALUE rb_drizzle_con_initialize(VALUE self, VALUE rb_cDrizzle)
{
    net_drizzle_con_st *context;
    net_drizzle_st *dri_st;
    Data_Get_Struct(self, net_drizzle_con_st, context);
    Data_Get_Struct(rb_cDrizzle, net_drizzle_st, dri_st);

    if ( ( context->con = drizzle_con_create(dri_st->drizzle, NULL) ) == NULL ) {
        rb_sys_fail("Failed to alloc drizzle_con_create.");
    }

    rb_ivar_set(self, rb_intern("@drizzle"), rb_cDrizzle);

    return self;
}

VALUE rb_drizzle_con_add_options(VALUE self, VALUE options)
{
    net_drizzle_con_st *context;
    Data_Get_Struct(self, net_drizzle_con_st, context);
    drizzle_con_add_options(context->con, FIX2INT(options));

    return self;
}

VALUE rb_drizzle_con_set_db(VALUE self, VALUE database)
{
    net_drizzle_con_st *context;
    Data_Get_Struct(self, net_drizzle_con_st, context);
    drizzle_con_set_db(context->con, RSTRING_PTR(database));

    return self;
}

VALUE rb_drizzle_con_host(VALUE self)
{
    net_drizzle_con_st *context;
    Data_Get_Struct(self, net_drizzle_con_st, context);

    return rb_str_new2(drizzle_con_host(context->con));
}

VALUE rb_drizzle_con_port(VALUE self)
{
    net_drizzle_con_st *context;
    Data_Get_Struct(self, net_drizzle_con_st, context);

    return INT2FIX(drizzle_con_port(context->con));
}

VALUE rb_drizzle_con_set_tcp(VALUE self, VALUE host, VALUE port)
{
    net_drizzle_con_st *context;
    Data_Get_Struct(self, net_drizzle_con_st, context);

    drizzle_con_set_tcp(context->con, RSTRING_PTR(host), FIX2INT(port));

    return self;
}

VALUE rb_drizzle_con_query_add(VALUE self, VALUE query_str)
{
    net_drizzle_con_st *context;
    Data_Get_Struct(self, net_drizzle_con_st, context);
    VALUE mNet = rb_const_get(rb_cObject, rb_intern("Net")); 
    VALUE cDrizzle = rb_const_get(mNet, rb_intern("Drizzle")); 
    VALUE cQuery = rb_const_get(cDrizzle, rb_intern("Query"));
    VALUE rb_query = rb_funcall(cQuery, rb_intern("allocate"), 0);

    net_drizzle_query_st *query;
    Data_Get_Struct(rb_query, net_drizzle_query_st, query);
    query->query = NULL;
    drizzle_result_st *result = NULL;
    drizzle_query_st *ret;
    ret = drizzle_query_add(
        context->con->drizzle, 
        query->query, 
        context->con, 
        result, 
        RSTRING_PTR(query_str), 
        RSTRING_LEN(query_str),
        (drizzle_query_options_t)0,
        NULL
    );
    if ( ret == NULL ) {
        rb_sys_fail("failed to create query instance.");
    }
    query->query = ret;

    return rb_query;

}

VALUE rb_drizzle_con_clone(VALUE self)
{
    net_drizzle_con_st *context;
    net_drizzle_con_st *copy;
    Data_Get_Struct(self, net_drizzle_con_st, context);
    VALUE mNet = rb_const_get(rb_cObject, rb_intern("Net")); 
    VALUE cDrizzle = rb_const_get(mNet, rb_intern("Drizzle")); 
    VALUE cConnection = rb_const_get(cDrizzle, rb_intern("Connection"));

    VALUE rb_query = rb_funcall(cConnection, rb_intern("allocate"), 0);
    Data_Get_Struct(rb_query, net_drizzle_con_st, copy);

    if ( ( copy->con = drizzle_con_clone(context->con->drizzle, NULL, context->con) ) == NULL ) {
        rb_sys_fail("clone failed");
    }

    return rb_query;
}

static void rb_drizzle_query_free(net_drizzle_query_st *query)
{
    if ( query->query != NULL ) {
        // FIXME: drizzle_free also free query memory. So rely it.
        // drizzle_query_free(query->query);
        query->query = NULL;
    }
    ruby_xfree(query);
}

static VALUE rb_drizzle_query_alloc(VALUE klass)
{
    VALUE self;
    net_drizzle_query_st *context;
    self = Data_Make_Struct(klass, net_drizzle_query_st, 0, rb_drizzle_query_free, context);

    return self;
}

VALUE rb_drizzle_query_initialize(VALUE self, VALUE rb_cDrizzle)
{
    net_drizzle_query_st *context;
    net_drizzle_st *dri_st;
    drizzle_query_st *query;

    Data_Get_Struct(self, net_drizzle_query_st, context);
    Data_Get_Struct(rb_cDrizzle, net_drizzle_st, dri_st);

    query = NULL;
    if ( ( context->query = drizzle_query_create(dri_st->drizzle, query) ) == NULL ) {
        rb_sys_fail("Failed to alloc drizzle_query_create.");
    }

    return self;
}

VALUE rb_drizzle_query_set_con(VALUE self, VALUE con)
{
    net_drizzle_query_st *context;
    net_drizzle_con_st *conn_st;
    Data_Get_Struct(self, net_drizzle_query_st, context);
    Data_Get_Struct(con, net_drizzle_con_st, conn_st);

    drizzle_query_set_con(context->query, conn_st->con);

    return self;
}

VALUE rb_drizzle_query_error_code(VALUE self)
{
    net_drizzle_query_st *context;
    Data_Get_Struct(self, net_drizzle_query_st, context);
    int ret;

    if ( context->query->result == NULL ) {
        // hmm. It should raise some Exception?
        return Qnil;
    }

    ret = drizzle_result_error_code(context->query->result);
    return INT2FIX(ret);
}

VALUE rb_drizzle_query_row_next(VALUE self)
{
    net_drizzle_query_st *context;
    Data_Get_Struct(self, net_drizzle_query_st, context);

    if ( context->query->result == NULL ) {
        // hmm. It should raise some Exception?
        return Qnil;
    }

    drizzle_row_t row = drizzle_row_next(context->query->result);
    if ( row == NULL ) {
        return Qnil;
    }

    uint16_t cnt = drizzle_result_column_count(context->query->result);
    VALUE result = rb_ary_new2(cnt);
    int i;
    for( i = 0; i < cnt; i++ ) {
        if ( row[i] ) {
            // FIXME: hmm. it may not be "UTF-8" string.
            rb_ary_push(result, rb_utf8_str_new(row[i], strlen(row[i])));
        } else {
            rb_ary_push(result, Qnil);
        }
    }

    return result;
}

void Init_drizzle()
{
    VALUE mNet = rb_define_module("Net");
    VALUE cDrizzle = rb_define_class_under(mNet, "Drizzle", rb_cObject);
    rb_define_alloc_func(cDrizzle, rb_drizzle_alloc);
    rb_define_method(cDrizzle, "con_create", rb_drizzle_con_create, 0);
    rb_define_method(cDrizzle, "query_run_all", rb_drizzle_query_run_all, 0);

    VALUE cConnection = rb_define_class_under(cDrizzle, "Connection", rb_cObject);
    rb_define_alloc_func(cConnection, rb_drizzle_con_alloc);
    rb_define_method(cConnection, "initialize", rb_drizzle_con_initialize, 1);
    rb_define_method(cConnection, "add_options", rb_drizzle_con_add_options, 1);
    rb_define_method(cConnection, "set_db", rb_drizzle_con_set_db, 1);
    rb_define_method(cConnection, "host", rb_drizzle_con_host, 0);
    rb_define_method(cConnection, "port", rb_drizzle_con_port, 0);
    rb_define_method(cConnection, "set_tcp", rb_drizzle_con_set_tcp, 2);
    rb_define_method(cConnection, "query_add", rb_drizzle_con_query_add, 1);
    rb_define_method(cConnection, "clone", rb_drizzle_con_clone, 0);

    VALUE mOptions = rb_define_module_under(cConnection, "Options");
    rb_define_const(mOptions, "NONE", INT2FIX(DRIZZLE_CON_NONE));
    rb_define_const(mOptions, "ALLOCATED", INT2FIX(DRIZZLE_CON_ALLOCATED));
    rb_define_const(mOptions, "MYSQL", INT2FIX((DRIZZLE_CON_MYSQL)));
    rb_define_const(mOptions, "RAW_PACKET", INT2FIX(DRIZZLE_CON_RAW_PACKET));
    rb_define_const(mOptions, "RAW_SCRAMBLE", INT2FIX(DRIZZLE_CON_RAW_SCRAMBLE));
    rb_define_const(mOptions, "READY", INT2FIX(DRIZZLE_CON_READY));
    rb_define_const(mOptions, "NO_RESULT_READ", INT2FIX(DRIZZLE_CON_NO_RESULT_READ));
    rb_define_const(mOptions, "IO_READY", INT2FIX(DRIZZLE_CON_IO_READY));

    VALUE cQuery = rb_define_class_under(cDrizzle, "Query", rb_cObject);
    rb_define_alloc_func(cQuery, rb_drizzle_query_alloc);
    rb_define_method(cQuery, "initialize", rb_drizzle_query_initialize, 1);
    rb_define_method(cQuery, "set_con", rb_drizzle_query_set_con, 1);
    rb_define_method(cQuery, "error_code", rb_drizzle_query_error_code, 0);
    rb_define_method(cQuery, "row_next", rb_drizzle_query_row_next, 0);

}

