#include <ruby.h>
#include "drizzle.h"

static void rb_drizzle_free(net_drizzle_st *context)
{
    drizzle_free(context->drizzle);
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

void Init_drizzle()
{
    VALUE mNet = rb_define_module("Net");
    VALUE cDrizzle = rb_define_class_under(mNet, "Drizzle", rb_cObject);
    rb_define_alloc_func(cDrizzle, rb_drizzle_alloc);
    rb_define_method(cDrizzle, "con_create", rb_drizzle_con_create, 0);

    VALUE cConnection = rb_define_class_under(cDrizzle, "Connection", rb_cObject);
    rb_define_alloc_func(cConnection, rb_drizzle_con_alloc);
    rb_define_method(cConnection, "initialize", rb_drizzle_con_initialize, 1);
    rb_define_method(cConnection, "add_options", rb_drizzle_con_add_options, 1);
    rb_define_method(cConnection, "set_db", rb_drizzle_con_set_db, 1);
    rb_define_method(cConnection, "host", rb_drizzle_con_host, 0);
    rb_define_method(cConnection, "port", rb_drizzle_con_port, 0);
    rb_define_method(cConnection, "set_tcp", rb_drizzle_con_set_tcp, 2);

    VALUE mOptions = rb_define_module_under(cConnection, "Options");
    rb_define_const(mOptions, "NONE", INT2FIX(DRIZZLE_CON_NONE));
    rb_define_const(mOptions, "ALLOCATED", INT2FIX(DRIZZLE_CON_ALLOCATED));
    rb_define_const(mOptions, "MYSQL", INT2FIX((DRIZZLE_CON_MYSQL)));
    rb_define_const(mOptions, "RAW_PACKET", INT2FIX(DRIZZLE_CON_RAW_PACKET));
    rb_define_const(mOptions, "RAW_SCRAMBLE", INT2FIX(DRIZZLE_CON_RAW_SCRAMBLE));
    rb_define_const(mOptions, "READY", INT2FIX(DRIZZLE_CON_READY));
    rb_define_const(mOptions, "NO_RESULT_READ", INT2FIX(DRIZZLE_CON_NO_RESULT_READ));
    rb_define_const(mOptions, "IO_READY", INT2FIX(DRIZZLE_CON_IO_READY));
}

