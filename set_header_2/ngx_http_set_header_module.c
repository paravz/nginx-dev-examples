
/*
 * Copyright (C) Nginx, Inc.
 */


#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>


typedef struct {
    ngx_str_t     name;
    ngx_str_t     value;
} ngx_http_set_header_entry_t;


/* location configuration */

typedef struct {
    ngx_array_t  *entries;
} ngx_http_set_header_loc_conf_t;


static ngx_int_t ngx_http_set_header_filter(ngx_http_request_t *r);
static void *ngx_http_set_header_create_loc_conf(ngx_conf_t *cf);
static char *ngx_http_set_header_merge_loc_conf(ngx_conf_t *cf, void *parent,
    void *child);
static char *ngx_http_set_header(ngx_conf_t *cf, ngx_command_t *cmd,
    void *conf);
static ngx_int_t ngx_http_set_header_init(ngx_conf_t *cf);


static ngx_command_t  ngx_http_set_header_commands[] = {

    { ngx_string("set_header"),
      NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_TAKE2,
      ngx_http_set_header,
      NGX_HTTP_LOC_CONF_OFFSET,
      0,
      NULL },

      ngx_null_command
};


static ngx_http_module_t  ngx_http_set_header_module_ctx = {
    NULL,                                  /* preconfiguration */
    ngx_http_set_header_init,              /* postconfiguration */

    NULL,                                  /* create main configuration */
    NULL,                                  /* init main configuration */

    NULL,                                  /* create server configuration */
    NULL,                                  /* merge server configuration */

    ngx_http_set_header_create_loc_conf,   /* create location configuration */
    ngx_http_set_header_merge_loc_conf     /* merge location configuration */
};


ngx_module_t  ngx_http_set_header_module = {
    NGX_MODULE_V1,
    &ngx_http_set_header_module_ctx,       /* module context */
    ngx_http_set_header_commands,          /* module directives */
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


/* next header filter in chain */

static ngx_http_output_header_filter_pt  ngx_http_next_header_filter;


/* header filter handler */

static ngx_int_t
ngx_http_set_header_filter(ngx_http_request_t *r)
{
    ngx_uint_t                       i;
    ngx_table_elt_t                 *h;
    ngx_http_set_header_entry_t     *entry;
    ngx_http_set_header_loc_conf_t  *slcf;

    ngx_log_debug0(NGX_LOG_DEBUG_HTTP, r->connection->log, 0,
                   "http set_header handler");

    slcf = ngx_http_get_module_loc_conf(r, ngx_http_set_header_module);

    /*
     * if no headers are defined for the location,
     * proceed to the next header filter in chain
     */

    if (slcf->entries == NULL) {
        return ngx_http_next_header_filter(r);
    }

    /* iterate over all headers in the location */

    entry = slcf->entries->elts;

    for (i = 0; i < slcf->entries->nelts; i++) {

        /* add header to output */

        h = ngx_list_push(&r->headers_out.headers);
        if (h == NULL) {
            return NGX_ERROR;
        }

        h->hash = 1;
        h->key = entry[i].name;
        h->value = entry[i].value;
    }

    /* proceed to the next handler in chain */

    return ngx_http_next_header_filter(r);
}


static void *
ngx_http_set_header_create_loc_conf(ngx_conf_t *cf)
{
    ngx_http_set_header_loc_conf_t  *conf;

    conf = ngx_pcalloc(cf->pool, sizeof(ngx_http_set_header_loc_conf_t));
    if (conf == NULL) {
        return NULL;
    }

    /*
     * set by ngx_pcalloc():
     *
     *     conf->entries = NULL;
     */

    return conf;
}


static char *
ngx_http_set_header_merge_loc_conf(ngx_conf_t *cf, void *parent, void *child)
{
    ngx_http_set_header_loc_conf_t *prev = parent;
    ngx_http_set_header_loc_conf_t *conf = child;

    if (conf->entries == NULL) {
        conf->entries = prev->entries;
    }

    return NGX_CONF_OK;
}


static char *
ngx_http_set_header(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_http_set_header_loc_conf_t *slcf = conf;

    ngx_str_t                    *value;
    ngx_http_set_header_entry_t  *entry;

    /* create array if missing */

    if (slcf->entries == NULL) {
        slcf->entries = ngx_array_create(cf->pool, 4,
                                         sizeof(ngx_http_set_header_entry_t));
        if (slcf->entries == NULL) {
            return NGX_CONF_ERROR;
        }
    }

    /* add new array entry */

    entry = ngx_array_push(slcf->entries);
    if (entry == NULL) {
        return NGX_CONF_ERROR;
    }

    value = cf->args->elts;

    /* set name from argument #1 */

    entry->name = value[1];

    /* value from argument #2 */

    entry->value = value[2];

    return NGX_CONF_OK;
}


static ngx_int_t
ngx_http_set_header_init(ngx_conf_t *cf)
{
    /* install handler in header filter chain */

    ngx_http_next_header_filter = ngx_http_top_header_filter;
    ngx_http_top_header_filter = ngx_http_set_header_filter;

    return NGX_OK;
}
