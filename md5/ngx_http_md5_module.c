
/*
 * Copyright (C) Nginx, Inc.
 */


#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>
#include <ngx_md5.h>


static ngx_int_t ngx_http_md5_variable(ngx_http_request_t *r,
    ngx_http_variable_value_t *v, uintptr_t data);
static char *ngx_http_md5(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);


static ngx_command_t  ngx_http_md5_commands[] = {

    { ngx_string("md5"),
      NGX_HTTP_MAIN_CONF|NGX_CONF_TAKE2,
      ngx_http_md5,
      0,
      0,
      NULL },

      ngx_null_command
};


static ngx_http_module_t  ngx_http_md5_module_ctx = {
    NULL,                                  /* preconfiguration */
    NULL,                                  /* postconfiguration */

    NULL,                                  /* create main configuration */
    NULL,                                  /* init main configuration */

    NULL,                                  /* create server configuration */
    NULL,                                  /* merge server configuration */

    NULL,                                  /* create location configuration */
    NULL                                   /* merge location configuration */
};


ngx_module_t  ngx_http_md5_module = {
    NGX_MODULE_V1,
    &ngx_http_md5_module_ctx,              /* module context */
    ngx_http_md5_commands,                 /* module directives */
    NGX_HTTP_MODULE,                       /* module type */
    NULL,                                  /* init master */
    NULL,                                  /* init module */
    NULL,                                  /* init process */
    NULL,                                  /* init thread */
    NULL,                                  /* exit thread */
    NULL,                                  /* exit process */
    NULL,                                  /* exit master */
    NGX_MODULE_V1_PADDING
};


static ngx_int_t
ngx_http_md5_variable(ngx_http_request_t *r, ngx_http_variable_value_t *v,
    uintptr_t data)
{
    ngx_http_complex_value_t *cv = (ngx_http_complex_value_t *) data;

    u_char     *p;
    ngx_md5_t   md5;
    ngx_str_t   value;
    u_char      md5_buf[16];

    ngx_log_debug0(NGX_LOG_DEBUG_HTTP, r->connection->log, 0,
                   "http md5 variable handler");

    /* evaluate complex value */

    if (ngx_http_complex_value(r, cv, &value) != NGX_OK) {
        return NGX_ERROR;
    }

    /* compute md5 */

    ngx_md5_init(&md5);
    ngx_md5_update(&md5, value.data, value.len);
    ngx_md5_final(md5_buf, &md5);

    p = ngx_pnalloc(r->pool, 32);
    if (p == NULL) {
        return NGX_ERROR;
    }

    ngx_hex_dump(p, md5_buf, 16);

    /* set variable value */

    v->len = 32;
    v->valid = 1;
    v->no_cacheable = 0;
    v->not_found = 0;
    v->data = p;

    return NGX_OK;
}


static char *
ngx_http_md5(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_str_t                         *value;
    ngx_http_variable_t               *var;
    ngx_http_complex_value_t          *cv;
    ngx_http_compile_complex_value_t   ccv;

    value = cf->args->elts;

    if (value[1].data[0] != '$') {
        ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                           "invalid variable name \"%V\"", &value[1]);
        return NGX_CONF_ERROR;
    }

    value[1].len--;
    value[1].data++;

    /* compile complex value from the argument */

    cv = ngx_palloc(cf->pool, sizeof(ngx_http_complex_value_t));
    if (cv == NULL) {
        return NGX_CONF_ERROR;
    }

    ngx_memzero(&ccv, sizeof(ngx_http_compile_complex_value_t));

    ccv.cf = cf;
    ccv.value = &value[2];
    ccv.complex_value = cv;

    if (ngx_http_compile_complex_value(&ccv) != NGX_OK) {
        return NGX_CONF_ERROR;
    }

    /* add variable */

    var = ngx_http_add_variable(cf, &value[1], 0);
    if (var == NULL) {
        return NGX_CONF_ERROR;
    }

    var->get_handler = ngx_http_md5_variable;
    var->data = (uintptr_t) cv;

    return NGX_CONF_OK;
}
