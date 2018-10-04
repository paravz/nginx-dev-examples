
/*
 * Copyright (C) Nginx, Inc.
 */


#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>


typedef struct {
    ngx_str_t  text;
} ngx_http_append_loc_conf_t;


static ngx_int_t ngx_http_append_header_filter(ngx_http_request_t *r);
static ngx_int_t ngx_http_append_body_filter(ngx_http_request_t *r,
    ngx_chain_t *in);
static void *ngx_http_append_create_loc_conf(ngx_conf_t *cf);
static char *ngx_http_append_merge_loc_conf(ngx_conf_t *cf, void *parent,
    void *child);
static ngx_int_t ngx_http_append_init(ngx_conf_t *cf);


static ngx_command_t  ngx_http_append_commands[] = {

    { ngx_string("append"),
      NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_str_slot,
      NGX_HTTP_LOC_CONF_OFFSET,
      offsetof(ngx_http_append_loc_conf_t, text),
      NULL },

      ngx_null_command
};


static ngx_http_module_t  ngx_http_append_module_ctx = {
    NULL,                                  /* preconfiguration */
    ngx_http_append_init,                  /* postconfiguration */

    NULL,                                  /* create main configuration */
    NULL,                                  /* init main configuration */

    NULL,                                  /* create server configuration */
    NULL,                                  /* merge server configuration */

    ngx_http_append_create_loc_conf,       /* create location configuration */
    ngx_http_append_merge_loc_conf         /* merge location configuration */
};


ngx_module_t  ngx_http_append_module = {
    NGX_MODULE_V1,
    &ngx_http_append_module_ctx,           /* module context */
    ngx_http_append_commands,              /* module directives */
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


/* next header and body filters in chain */

static ngx_http_output_header_filter_pt  ngx_http_next_header_filter;
static ngx_http_output_body_filter_pt    ngx_http_next_body_filter;


static ngx_int_t
ngx_http_append_header_filter(ngx_http_request_t *r)
{
    ngx_http_append_loc_conf_t  *plcf;

    ngx_log_debug0(NGX_LOG_DEBUG_HTTP, r->connection->log, 0,
                   "http append header handler");

    plcf = ngx_http_get_module_loc_conf(r, ngx_http_append_module);

    if (plcf->text.len == 0) {
        return ngx_http_next_header_filter(r);
    }

    /* reset content length */
    ngx_http_clear_content_length(r);

    /* disable ranges */
    ngx_http_clear_accept_ranges(r);

    /* clear etag */
    ngx_http_clear_etag(r);

    return ngx_http_next_header_filter(r);
}


static ngx_int_t
ngx_http_append_body_filter(ngx_http_request_t *r, ngx_chain_t *in)
{
    ngx_buf_t                   *b;
    ngx_int_t                    rc;
    ngx_uint_t                   last;
    ngx_chain_t                  out, *cl;
    ngx_http_append_loc_conf_t  *plcf;

    ngx_log_debug0(NGX_LOG_DEBUG_HTTP, r->connection->log, 0,
                   "http append body handler");

    plcf = ngx_http_get_module_loc_conf(r, ngx_http_append_module);

    if (plcf->text.len == 0) {
        return ngx_http_next_body_filter(r, in);
    }

    /* iterate over the buffers and find last_buf */

    last = 0;

    for (cl = in; cl; cl = cl->next) {
        if (cl->buf->last_buf) {
            cl->buf->last_buf = 0;
            cl->buf->sync = 1;
            last = 1;
        }
    }

    rc = ngx_http_next_body_filter(r, in);

    if (rc == NGX_ERROR || !last) {
        return rc;
    }

    /* output text */

    b = ngx_calloc_buf(r->pool);
    if (b == NULL) {
        return NGX_ERROR;
    }

    b->pos = plcf->text.data;
    b->last = plcf->text.data + plcf->text.len;
    b->memory = 1;
    b->last_buf = 1;

    out.buf = b;
    out.next = NULL;

    return ngx_http_next_body_filter(r, &out);
}


static void *
ngx_http_append_create_loc_conf(ngx_conf_t *cf)
{
    ngx_http_append_loc_conf_t  *conf;

    conf = ngx_pcalloc(cf->pool, sizeof(ngx_http_append_loc_conf_t));
    if (conf == NULL) {
        return NULL;
    }

    /*
     * set by ngx_pcalloc():
     *
     *     conf->text = { 0, NULL };
     */

    return conf;
}


static char *
ngx_http_append_merge_loc_conf(ngx_conf_t *cf, void *parent, void *child)
{
    ngx_http_append_loc_conf_t *prev = parent;
    ngx_http_append_loc_conf_t *conf = child;

    ngx_conf_merge_str_value(conf->text, prev->text, "");

    return NGX_CONF_OK;
}


static ngx_int_t
ngx_http_append_init(ngx_conf_t *cf)
{
    /* install handler in header filter chain */

    ngx_http_next_header_filter = ngx_http_top_header_filter;
    ngx_http_top_header_filter = ngx_http_append_header_filter;

    /* install handler in body filter chain */

    ngx_http_next_body_filter = ngx_http_top_body_filter;
    ngx_http_top_body_filter = ngx_http_append_body_filter;

    return NGX_OK;
}
