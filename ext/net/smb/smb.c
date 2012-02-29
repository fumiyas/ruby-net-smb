#include <ruby.h>
#include <libsmbclient.h>

struct smbcctx {
  SMBCCTX       *ctx;
  VALUE         auth_callback;
};

#define FALSE_P(value)  (NIL_P(value) || (value) == Qfalse)
#define TRUE_P(value)   !FALSE_P(value)

#define bool2value(b)   ((b) ? Qtrue : Qfalse)
#define value2bool(v)   ((b) ? Qtrue : Qfalse)

#define get_smbcctx(self) \
  struct smbcctx *smbcctx; \
  Data_Get_Struct((self), struct smbcctx, smbcctx);

static VALUE eRuntimeError;

/* ====================================================================== */

static void smbcctx_gc_mark(struct smbcctx *smbcctx)
{
  rb_gc_mark(smbcctx->auth_callback);
}

static void smbcctx_free(struct smbcctx *smbcctx)
{
  smbc_free_context(smbcctx->ctx, 1);
}

static VALUE rb_smbcctx_alloc(VALUE klass)
{
  struct smbcctx *smbcctx = ALLOC(struct smbcctx);

  memset(smbcctx, 0, sizeof(struct smbcctx));

  /* FIXME: Unset $HOME to ignore $HOME/.smb/smb.conf */
  smbcctx->ctx = smbc_new_context();
  if (smbcctx->ctx == NULL) {
    rb_sys_fail("Cannot create SMBCCTX");
  }

  smbcctx->auth_callback = Qnil;

  return Data_Wrap_Struct(klass, smbcctx_gc_mark, smbcctx_free, smbcctx);
}

static VALUE rb_smbcctx_initialize(VALUE self)
{
  get_smbcctx(self);

  smbc_setDebug(smbcctx->ctx, 0);
  smbc_setOptionDebugToStderr(smbcctx->ctx, (smbc_bool)1);

  return self;
}

static VALUE rb_smbcctx_debug_get(VALUE self)
{
  get_smbcctx(self);

  return INT2NUM(smbc_getDebug(smbcctx->ctx));
}

static VALUE rb_smbcctx_debug_set(VALUE self, VALUE debug)
{
  get_smbcctx(self);

  smbc_setDebug(smbcctx->ctx, NUM2INT(debug));

  return debug;
}

static VALUE rb_smbcctx_use_kerberos_get(VALUE self)
{
  get_smbcctx(self);

  return smbc_getOptionUseKerberos(smbcctx->ctx) ? Qtrue : Qfalse;
}

static VALUE rb_smbcctx_use_kerberos_set(VALUE self, VALUE flag)
{
  get_smbcctx(self);

  smbc_setOptionUseKerberos(smbcctx->ctx, (smbc_bool)(TRUE_P(flag) ? 1 : 0));

  return flag;
}

/* ====================================================================== */

void Init_smb(void)
{
  VALUE rb_mNet = rb_define_module("Net");
  VALUE rb_mSMB = rb_define_module_under(rb_mNet, "SMB");
  VALUE rb_cSMBCCTX = rb_define_class_under(rb_mNet, "SMBCCTX", rb_cObject);

  rb_define_alloc_func(rb_cSMBCCTX, rb_smbcctx_alloc);
  rb_define_method(rb_cSMBCCTX, "initialize", rb_smbcctx_initialize, 0);
  rb_define_method(rb_cSMBCCTX, "debug", rb_smbcctx_debug_get, 0);
  rb_define_method(rb_cSMBCCTX, "debug=", rb_smbcctx_debug_set, 1);

  //eRuntimeError = rb_define_class_under(mSMB, "Net::SMB::RuntimeError", rb_eRuntimeError);
}

