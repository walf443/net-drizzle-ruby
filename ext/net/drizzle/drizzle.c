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

void Init_drizzle()
{
    VALUE mNet = rb_define_module("Net");
    VALUE cDrizzle = rb_define_class_under(mNet, "Drizzle", rb_cObject);
    rb_define_alloc_func(cDrizzle, rb_drizzle_alloc);
}

