/*
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "ruby.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include "mtbl.h"

static VALUE rb_cMTBL;
static VALUE rb_cMTBLReader;
static VALUE rb_cMTBLIterator;
static VALUE rb_cMTBLWriter;
static VALUE rb_cMTBLSorter;
static VALUE rb_cMTBLUtil;
static VALUE rb_cMTBLVersion;

#define MTBL_VERSION "1.0.0"

typedef struct {
    struct mtbl_reader *r;
} rbmtbl_reader_t;

typedef struct {
    struct mtbl_iter *it;
} rbmtbl_iterator_t;

typedef struct {
    struct mtbl_writer *w;
    struct mtbl_writer_options *o;
} rbmtbl_writer_t;

typedef struct {
    struct mtbl_sorter *s;
    struct mtbl_sorter_options *o;
} rbmtbl_sorter_t;



/*
 * Iterator
 */


static VALUE rbmtbl_iterator_free(rbmtbl_iterator_t *iterator) {
    mtbl_iter_destroy(&iterator->it);
    free(iterator);
    return Qnil;
}

static VALUE rbmtbl_iterator_alloc(VALUE class) {
    rbmtbl_iterator_t *iterator = malloc(sizeof(rbmtbl_iterator_t));
    memset(iterator, 0, sizeof(rbmtbl_iterator_t));
    return Data_Wrap_Struct(class, 0, rbmtbl_iterator_free, iterator);
}


static VALUE rbmtbl_iterator_initialize(VALUE self, VALUE c_reader) {
    rbmtbl_iterator_t *iterator;
    rbmtbl_reader_t *reader;
    Data_Get_Struct(self, rbmtbl_iterator_t, iterator);
    // TODO: Verify that c_reader is a MTBL::Reader
    Data_Get_Struct(c_reader, rbmtbl_reader_t, reader);
    iterator->it = mtbl_source_iter(mtbl_reader_source(reader->r));
    return self;
}

static VALUE rbmtbl_iterator_next(VALUE self) {
    const uint8_t *key, *val;
    size_t len_key, len_val;
    VALUE r_key;
    VALUE r_val;
    VALUE r_arr;
    rbmtbl_iterator_t *iterator;
    Data_Get_Struct(self, rbmtbl_iterator_t, iterator);

    while (mtbl_iter_next(iterator->it, &key, &len_key, &val, &len_val)) {
        r_key = rb_usascii_str_new((const char *)key, len_key);
        r_val = rb_usascii_str_new((const char *)val, len_val);

        r_arr = rb_ary_new2(2);
        rb_ary_push(r_arr, r_key);
        rb_ary_push(r_arr, r_val);

        if(rb_block_given_p()) {
            rb_yield(r_arr);
        } else {
            return r_arr;
        }
    }
    return Qnil;
}

/*
 * Reader
 */

static VALUE rbmtbl_reader_free(rbmtbl_reader_t *reader) {
    if(reader->r) {
      mtbl_reader_destroy(&reader->r);
      reader->r = NULL;
    }
    free(reader);
    return Qnil;
}

static VALUE rbmtbl_reader_alloc(VALUE class) {
    rbmtbl_reader_t *reader = malloc(sizeof(rbmtbl_reader_t));
    memset(reader, 0, sizeof(rbmtbl_reader_t));
    return Data_Wrap_Struct(class, 0, rbmtbl_reader_free, reader);
}

static VALUE rbmtbl_reader_initialize(VALUE self, VALUE fname) {
    rbmtbl_reader_t *reader;
    Data_Get_Struct(self, rbmtbl_reader_t, reader);

    if (TYPE(fname) != T_STRING) {
        rb_raise(rb_eArgError, "File name must be a string");
        return Qnil;
    }

    reader->r = mtbl_reader_init(StringValueCStr(fname), NULL);
    if (reader->r == NULL) {
        rb_raise(rb_eRuntimeError, "Failed to open %s", StringValueCStr(fname));
        return (false);
    }
    return self;
}

static VALUE rbmtbl_reader_iterator(VALUE self) {
    VALUE argv[1];
    argv[0] = self;
    return rb_class_new_instance(1, argv, rb_cMTBLIterator);
}

static VALUE rbmtbl_reader_get(VALUE self, VALUE key) {
    VALUE iter = rb_obj_alloc(rb_cMTBLIterator);
    rbmtbl_reader_t *reader;
    rbmtbl_iterator_t *iterator;
    Data_Get_Struct(self, rbmtbl_reader_t, reader);
    Data_Get_Struct(iter, rbmtbl_iterator_t, iterator);

    if (TYPE(key) != T_STRING) {
        rb_raise(rb_eArgError, "Key must be a string");
        return Qnil;
    }

    iterator->it = mtbl_source_get(mtbl_reader_source(reader->r),
        (const uint8_t *) RSTRING_PTR(key), RSTRING_LEN(key));
    return iter;
}

static VALUE rbmtbl_reader_get_prefix(VALUE self, VALUE prefix) {
    VALUE iter = rb_obj_alloc(rb_cMTBLIterator);
    rbmtbl_reader_t *reader;
    rbmtbl_iterator_t *iterator;
    Data_Get_Struct(self, rbmtbl_reader_t, reader);
    Data_Get_Struct(iter, rbmtbl_iterator_t, iterator);

    if (TYPE(prefix) != T_STRING) {
        rb_raise(rb_eArgError, "Prefix must be a string");
        return Qnil;
    }

    iterator->it = mtbl_source_get_prefix(mtbl_reader_source(reader->r),
        (const uint8_t *) RSTRING_PTR(prefix), RSTRING_LEN(prefix));
    return iter;
}

static VALUE rbmtbl_reader_get_range(VALUE self, VALUE kstart, VALUE kend) {
    VALUE iter = rb_obj_alloc(rb_cMTBLIterator);
    rbmtbl_reader_t *reader;
    rbmtbl_iterator_t *iterator;
    Data_Get_Struct(self, rbmtbl_reader_t, reader);
    Data_Get_Struct(iter, rbmtbl_iterator_t, iterator);

    if (TYPE(kstart) != T_STRING) {
        rb_raise(rb_eArgError, "Range start must be a string");
        return Qnil;
    }

    if (TYPE(kend) != T_STRING) {
        rb_raise(rb_eArgError, "Range stop must be a string");
        return Qnil;
    }

    iterator->it = mtbl_source_get_range(mtbl_reader_source(reader->r),
        (const uint8_t *) RSTRING_PTR(kstart), RSTRING_LEN(kstart),
        (const uint8_t *) RSTRING_PTR(kend), RSTRING_LEN(kend));
    return iter;
}


/*
 * Writer
 */

void rbmtbl_writer_close_handles(rbmtbl_writer_t *writer) {
    if(writer->w) {
      mtbl_writer_destroy(&writer->w);
      writer->w = NULL;
    }
    if (writer->o) {
      mtbl_writer_options_destroy(&writer->o);
      writer->o = NULL;
    }
}
static VALUE rbmtbl_writer_free(rbmtbl_writer_t *writer) {
    rbmtbl_writer_close_handles(writer);
    free(writer);
    return Qnil;
}

static VALUE rbmtbl_writer_alloc(VALUE class) {
    rbmtbl_writer_t *writer = malloc(sizeof(rbmtbl_writer_t));
    memset(writer, 0, sizeof(rbmtbl_writer_t));
    writer->o = mtbl_writer_options_init();
    return Data_Wrap_Struct(class, 0, rbmtbl_writer_free, writer);
}

static VALUE rbmtbl_writer_initialize(int argc, VALUE *argv, VALUE self) {
    rbmtbl_writer_t *writer;
    struct stat ss;
    int scanc;
    VALUE fname, ctype, bsize, rinterval;

    Data_Get_Struct(self, rbmtbl_writer_t, writer);

    scanc = rb_scan_args(argc, argv, "13", &fname, &ctype, &bsize, &rinterval);

    if (scanc > 1) {
        if (TYPE(ctype) != T_FIXNUM) {
          rb_raise(rb_eArgError, "Compression type should be a constant (COMPRESSION_NONE, etc)");
          return Qnil;
        }
        if (FIX2INT(ctype) < MTBL_COMPRESSION_NONE || FIX2INT(ctype) > MTBL_COMPRESSION_LZ4HC) {
            rb_raise(rb_eArgError, "Invalid compression type: %d", FIX2INT(ctype));
            return Qnil;
        }
        mtbl_writer_options_set_compression(writer->o, FIX2INT(ctype));
    }

    if (scanc > 2) {
        if (TYPE(bsize) != T_FIXNUM) {
            rb_raise(rb_eArgError, "Block size should be an integer");
            return Qnil;
        }
      mtbl_writer_options_set_block_size(writer->o, FIX2INT(bsize));
    }

    if (scanc > 3) {
        if (TYPE(rinterval) != T_FIXNUM) {
            rb_raise(rb_eArgError, "Restart interval should be an integer");
            return Qnil;
        }
      mtbl_writer_options_set_block_restart_interval(writer->o, FIX2INT(rinterval));
    }

    if (TYPE(fname) != T_STRING) {
        rb_raise(rb_eArgError, "File name must be a string");
        return Qnil;
    }

    if (! stat(StringValueCStr(fname), &ss)) {
        rb_raise(rb_eArgError, "File already exists: %s", StringValueCStr(fname));
        return Qnil;
    }

    writer->w = mtbl_writer_init(StringValueCStr(fname), writer->o);
    if (writer->w == NULL) {
        rb_raise(rb_eRuntimeError, "Failed to open %s", StringValueCStr(fname));
        return Qnil;
    }
    return self;
}

static VALUE rbmtbl_writer_add(VALUE self, VALUE key, VALUE val) {
    rbmtbl_writer_t *writer;
    Data_Get_Struct(self, rbmtbl_writer_t, writer);
    if (! writer->w) {
        rb_raise(rb_eRuntimeError, "Failed to write key %s: writer closed", StringValueCStr(key));
        return Qnil;
    }

    if (TYPE(key) != T_STRING) {
        rb_raise(rb_eArgError, "Key must be a string");
        return Qnil;
    }

    if (TYPE(val) != T_STRING) {
        rb_raise(rb_eArgError, "Value must be a string");
        return Qnil;
    }

    if( mtbl_res_success != mtbl_writer_add(writer->w,
        (const uint8_t *) RSTRING_PTR(key), RSTRING_LEN(key),
        (const uint8_t *) RSTRING_PTR(val), RSTRING_LEN(val))) {
        rb_raise(rb_eRuntimeError, "Failed to write key %s, input must be presorted", StringValueCStr(key));
        return Qnil;
    }
    return Qtrue;
}

static VALUE rbmtbl_writer_close(VALUE self) {
    rbmtbl_writer_t *writer;
    Data_Get_Struct(self, rbmtbl_writer_t, writer);
    if (writer->w) {
        rbmtbl_writer_close_handles(writer);
        return Qtrue;
    } else {
        rb_raise(rb_eRuntimeError, "Writer is already closed");
        return Qfalse;
    }
}

/*
 * Sorter
 */

void rbmbtl_default_merge_func(void *merge_info,
                               uint8_t *key, size_t len_key,
                               uint8_t *val0, size_t len_val0,
                               uint8_t *val1, size_t len_val1,
                               uint8_t **merged_val, size_t * len_merged_val ) {
  // TODO: Allow Ruby callbacks for the merge
  // Choose the newer value by default
  rbmtbl_sorter_t *sorter = (rbmtbl_sorter_t *) merge_info;
  merged_val[0] = malloc(len_val1);
  *len_merged_val = len_val1;
  memcpy(merged_val[0], val1, len_val1);
}


void rbmtbl_sorter_close_handles(rbmtbl_sorter_t *sorter) {
    if(sorter->s) {
      mtbl_sorter_destroy(&sorter->s);
      sorter->s = NULL;
    }
    if (sorter->o) {
      mtbl_sorter_options_destroy(&sorter->o);
      sorter->o = NULL;
    }
}

static VALUE rbmtbl_sorter_free(rbmtbl_sorter_t *sorter) {
    rbmtbl_sorter_close_handles(sorter);
    free(sorter);
    return Qnil;
}

static VALUE rbmtbl_sorter_alloc(VALUE class) {
    rbmtbl_sorter_t *sorter = malloc(sizeof(rbmtbl_sorter_t));
    memset(sorter, 0, sizeof(rbmtbl_sorter_t));
    sorter->o = mtbl_sorter_options_init();
    return Data_Wrap_Struct(class, 0, rbmtbl_sorter_free, sorter);
}

static VALUE rbmtbl_sorter_initialize(int argc, VALUE *argv, VALUE self) {
    rbmtbl_sorter_t *sorter;
    struct stat ss;
    int scanc;
    VALUE mergef, tempd, maxm;

    Data_Get_Struct(self, rbmtbl_sorter_t, sorter);

    scanc = rb_scan_args(argc, argv, "03", &mergef, &tempd, &maxm);
    mtbl_sorter_options_set_merge_func(sorter->o, (mtbl_merge_func) rbmbtl_default_merge_func, (void *) sorter);

    if (scanc > 1 && mergef != Qnil) {
        // TODO: Implement the merge callback
    }

    if (scanc > 2 && tempd != Qnil) {
        if (TYPE(tempd) != T_STRING) {
            rb_raise(rb_eArgError, "Temporary directory should be a string");
            return Qnil;
        }
        if (stat(StringValueCStr(tempd), &ss)) {
            rb_raise(rb_eArgError, "Temporary directory does not exist: %s", StringValueCStr(tempd));
            return Qnil;
        }

        if (! S_ISDIR(ss.st_mode)) {
            rb_raise(rb_eArgError, "Path %s is not a directory", StringValueCStr(tempd));
            return Qnil;
        }
        mtbl_sorter_options_set_temp_dir(sorter->o, StringValueCStr(tempd));
    }

    if (scanc > 3 && maxm != Qnil) {
        mtbl_sorter_options_set_max_memory(sorter->o, NUM2ULONG(maxm));
    }

    // Verify that c_writer is a MTBL::Writer

    sorter->s = mtbl_sorter_init(sorter->o);
    if (sorter->s == NULL) {
        rb_raise(rb_eRuntimeError, "Failed to create sorter");
        return Qnil;
    }
    return self;
}

static VALUE rbmtbl_sorter_add(VALUE self, VALUE key, VALUE val) {
    rbmtbl_sorter_t *sorter;
    Data_Get_Struct(self, rbmtbl_sorter_t, sorter);
    if (! sorter->s) {
        rb_raise(rb_eRuntimeError, "Failed to write key %s: sorter closed", StringValueCStr(key));
        return Qnil;
    }

    if (TYPE(key) != T_STRING) {
        rb_raise(rb_eArgError, "Key must be a string");
        return Qnil;
    }

    if (TYPE(val) != T_STRING) {
        rb_raise(rb_eArgError, "Value must be a string");
        return Qnil;
    }

    if( mtbl_res_success != mtbl_sorter_add(sorter->s,
        (const uint8_t *) RSTRING_PTR(key), RSTRING_LEN(key),
        (const uint8_t *) RSTRING_PTR(val), RSTRING_LEN(val))) {
        rb_raise(rb_eRuntimeError, "Failed to write key %s, input must be presorted", StringValueCStr(key));
        return Qnil;
    }
    return Qtrue;
}

static VALUE rbmtbl_sorter_close(VALUE self) {
    rbmtbl_sorter_t *sorter;
    Data_Get_Struct(self, rbmtbl_sorter_t, sorter);
    if (sorter->s) {
        rbmtbl_sorter_close_handles(sorter);
        return Qtrue;
    } else {
        rb_raise(rb_eRuntimeError, "Sorter is already closed");
        return Qfalse;
    }
}

static VALUE rbmtbl_sorter_write(VALUE self, VALUE c_writer) {
    rbmtbl_sorter_t *sorter;
    rbmtbl_writer_t *writer;

    Data_Get_Struct(self, rbmtbl_sorter_t, sorter);

    // Verify MTBL::Writer is c_writer
    Data_Get_Struct(c_writer, rbmtbl_writer_t, writer);

    if (mtbl_res_success != mtbl_sorter_write(sorter->s, writer->w)) {
        rb_raise(rb_eRuntimeError, "Failed to write");
        return Qnil;
    }

    return Qtrue;
}

/*
 * Utils
 */

static VALUE rbmtbl_utils_metadata(VALUE class, VALUE fname) {
    int fd, ret;
    struct stat ss;
    struct mtbl_reader *r;
    const struct mtbl_metadata *m;
    mtbl_compression_type compression_algorithm;
    uint64_t data_block_size, count_entries, count_data_blocks, bytes_data_blocks;
    uint64_t bytes_index_block, bytes_keys, bytes_values, index_block_offset;
    double p_data, p_index, compactness;
    VALUE metadata = rb_hash_new();

    if (TYPE(fname) != T_STRING) {
        rb_raise(rb_eArgError, "File name must be a string");
        return Qnil;
    }

    fd = open(StringValueCStr(fname), O_RDONLY);
    if (fd < 0) {
        rb_raise(rb_eRuntimeError, "Unable to open file %s: %s", StringValueCStr(fname), strerror(errno));
        return Qnil;
    }

    ret = fstat(fd, &ss);
    if (ret < 0) {
        close(fd);
        rb_raise(rb_eRuntimeError, "Failed fstat on file %s: %s", StringValueCStr(fname), strerror(errno));
        return Qnil;
    }

    r = mtbl_reader_init_fd(fd, NULL);
    if (r == NULL) {
        close(fd);
        rb_raise(rb_eRuntimeError, "Unable to open file %s: mtbl_reader_init_fd()", StringValueCStr(fname));
        return Qnil;
    }

    m = mtbl_reader_metadata(r);

    data_block_size = mtbl_metadata_data_block_size(m);
    compression_algorithm = mtbl_metadata_compression_algorithm(m);
    count_entries = mtbl_metadata_count_entries(m);
    count_data_blocks = mtbl_metadata_count_data_blocks(m);
    bytes_data_blocks = mtbl_metadata_bytes_data_blocks(m);
    bytes_index_block = mtbl_metadata_bytes_index_block(m);
    bytes_keys = mtbl_metadata_bytes_keys(m);
    bytes_values = mtbl_metadata_bytes_values(m);
    index_block_offset = mtbl_metadata_index_block_offset(m);
    p_data = 100.0 * bytes_data_blocks / ss.st_size;
    p_index = 100.0 * bytes_index_block / ss.st_size;
    compactness = 100.0 * ss.st_size / (bytes_keys + bytes_values);

    rb_hash_aset(metadata, ID2SYM(rb_intern_const("filename")), fname);
    rb_hash_aset(metadata, ID2SYM(rb_intern_const("filesize")), LL2NUM((size_t) ss.st_size));
    rb_hash_aset(metadata, ID2SYM(rb_intern_const("index_block_offset")), LL2NUM(index_block_offset));
    rb_hash_aset(metadata, ID2SYM(rb_intern_const("index_bytes")), LL2NUM(bytes_index_block));
    rb_hash_aset(metadata, ID2SYM(rb_intern_const("index_bytes_pct")), DBL2NUM(p_index));
    rb_hash_aset(metadata, ID2SYM(rb_intern_const("data_block_bytes")), LL2NUM(bytes_data_blocks));
    rb_hash_aset(metadata, ID2SYM(rb_intern_const("data_block_bytes_pct")), DBL2NUM(p_data));
    rb_hash_aset(metadata, ID2SYM(rb_intern_const("data_block_size")), LL2NUM(data_block_size));
    rb_hash_aset(metadata, ID2SYM(rb_intern_const("data_block_count")), LL2NUM(count_data_blocks));
    rb_hash_aset(metadata, ID2SYM(rb_intern_const("entry_count")), LL2NUM(count_entries));
    rb_hash_aset(metadata, ID2SYM(rb_intern_const("key_bytes")), LL2NUM(bytes_keys));
    rb_hash_aset(metadata, ID2SYM(rb_intern_const("value_bytes")), LL2NUM(bytes_values));
    rb_hash_aset(metadata, ID2SYM(rb_intern_const("compression_algorithm")), rb_str_new2(mtbl_compression_type_to_str(compression_algorithm)));
    rb_hash_aset(metadata, ID2SYM(rb_intern_const("compactness")), DBL2NUM(compactness));

    mtbl_reader_destroy(&r);
    close(fd);

    return metadata;
}

void Init_mtbl() {
    rb_cMTBL = rb_define_class("MTBL", rb_cObject);
    rb_cMTBLReader = rb_define_class_under(rb_cMTBL, "Reader", rb_cObject);
    rb_cMTBLIterator = rb_define_class_under(rb_cMTBL, "Iterator", rb_cObject);
    rb_cMTBLWriter = rb_define_class_under(rb_cMTBL, "Writer", rb_cObject);
    rb_cMTBLSorter = rb_define_class_under(rb_cMTBL, "Sorter", rb_cObject);
    rb_cMTBLUtil = rb_define_class_under(rb_cMTBL, "Utils", rb_cObject);

    rb_cMTBLVersion = rb_str_new2((const char *)MTBL_VERSION);
    rb_define_const(rb_cMTBL, "Version", rb_cMTBLVersion);
    rb_define_const(rb_cMTBL, "COMPRESSION_NONE", INT2FIX(MTBL_COMPRESSION_NONE));
    rb_define_const(rb_cMTBL, "COMPRESSION_SNAPPY", INT2FIX(MTBL_COMPRESSION_SNAPPY));
    rb_define_const(rb_cMTBL, "COMPRESSION_ZLIB", INT2FIX(MTBL_COMPRESSION_ZLIB));
    rb_define_const(rb_cMTBL, "COMPRESSION_LZ4", INT2FIX(MTBL_COMPRESSION_LZ4));
    rb_define_const(rb_cMTBL, "COMPRESSION_LZ4HC", INT2FIX(MTBL_COMPRESSION_LZ4HC));

    rb_define_alloc_func(rb_cMTBLReader, rbmtbl_reader_alloc);
    rb_define_method(rb_cMTBLReader, "initialize", rbmtbl_reader_initialize, 1);
    rb_define_method(rb_cMTBLReader, "iterator", rbmtbl_reader_iterator, 0);
    rb_define_method(rb_cMTBLReader, "get", rbmtbl_reader_get, 1);
    rb_define_method(rb_cMTBLReader, "get_prefix", rbmtbl_reader_get_prefix, 1);
    rb_define_method(rb_cMTBLReader, "get_range", rbmtbl_reader_get_range, 2);

    rb_define_method(rb_cMTBLIterator, "initialize", rbmtbl_iterator_initialize, 1);
    rb_define_alloc_func(rb_cMTBLIterator, rbmtbl_iterator_alloc);
    rb_define_method(rb_cMTBLIterator, "next", rbmtbl_iterator_next, 0);
    rb_define_method(rb_cMTBLIterator, "each", rbmtbl_iterator_next, 0);

    rb_define_method(rb_cMTBLWriter, "initialize", rbmtbl_writer_initialize, -1);
    rb_define_alloc_func(rb_cMTBLWriter, rbmtbl_writer_alloc);
    rb_define_method(rb_cMTBLWriter, "add", rbmtbl_writer_add, 2);
    rb_define_method(rb_cMTBLWriter, "close", rbmtbl_writer_close, 0);

    rb_define_method(rb_cMTBLSorter, "initialize", rbmtbl_sorter_initialize, -1);
    rb_define_alloc_func(rb_cMTBLSorter, rbmtbl_sorter_alloc);
    rb_define_method(rb_cMTBLSorter, "add", rbmtbl_sorter_add, 2);
    rb_define_method(rb_cMTBLSorter, "write", rbmtbl_sorter_write, 1);
    rb_define_method(rb_cMTBLSorter, "close", rbmtbl_sorter_close, 0);

    rb_define_singleton_method(rb_cMTBLUtil, "metadata", rbmtbl_utils_metadata, 1);
}
