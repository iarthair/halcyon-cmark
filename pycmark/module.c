#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <cmark-gfm.h>
#include <cmark-gfm-extension_api.h>

cmark_syntax_extension *create_table_extension ();
cmark_syntax_extension *create_strikethrough_extension ();
cmark_syntax_extension *create_tagfilter_extension ();
cmark_syntax_extension *create_autolink_extension ();

/* https://docs.python.org/3.8/extending/newtypes_tutorial.html */
typedef struct _CMarkObject CMarkObject;
struct _CMarkObject
  {
    PyObject_HEAD

    cmark_node *document;
    int options;
  };

static void
cmark_dealloc (CMarkObject *self)
{
  cmark_node_free (self->document);

  Py_TYPE(self)->tp_free((PyObject *) self);
}

static int
cmark_init (CMarkObject *self, PyObject *args, PyObject *kwds)
{
  const char *text;
  Py_ssize_t len;
  cmark_parser *parser;
  cmark_syntax_extension *syntax_extension;

  if (!PyArg_ParseTuple (args, "s#", &text, &len))
    return -1;

  self->options =   CMARK_OPT_SMART
		  | CMARK_OPT_FOOTNOTES
		  | CMARK_OPT_STRIKETHROUGH_DOUBLE_TILDE
		  | CMARK_OPT_TABLE_PREFER_STYLE_ATTRIBUTES
		  | CMARK_OPT_FULL_INFO_STRING
		  ;

  parser =  cmark_parser_new (self->options);

  //syntax_extension = cmark_find_syntax_extension(argv[i]);
  syntax_extension = create_table_extension ();
  cmark_parser_attach_syntax_extension (parser, syntax_extension);
  syntax_extension = create_strikethrough_extension ();
  cmark_parser_attach_syntax_extension (parser, syntax_extension);
  syntax_extension = create_tagfilter_extension ();
  cmark_parser_attach_syntax_extension (parser, syntax_extension);
  syntax_extension = create_autolink_extension ();
  cmark_parser_attach_syntax_extension (parser, syntax_extension);

  cmark_parser_feed (parser, text, len);
  self->document = cmark_parser_finish (parser);
  cmark_parser_free (parser);

  return self->document != NULL ? 0 : -1;
}

static PyObject *
cmark_html (CMarkObject *self, PyObject *Py_UNUSED(ignored))
{
  PyObject *tail;
  char *text;

  text = cmark_render_html (self->document, self->options, NULL);
  tail = PyUnicode_FromString (text);
  free (text);
  return tail;
}

static PyMethodDef cmark_methods[] = {
    {
      "render_html",
      (PyCFunction) cmark_html,
      METH_NOARGS,
      ""
    },
    { NULL }
};

static PyTypeObject CMarkType =
  {
    PyVarObject_HEAD_INIT (NULL, 0)
    .tp_name = "cmark.CMark",
    .tp_doc = "Custom objects for GFM Commonmark",
    .tp_basicsize = sizeof (CMarkObject),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_new = PyType_GenericNew,
    .tp_init = (initproc) cmark_init,
    .tp_dealloc = (destructor) cmark_dealloc,
    //.tp_members = cmark_members,
    .tp_methods = cmark_methods,
  };

static struct PyModuleDef moduledef =
  {
    PyModuleDef_HEAD_INIT,
    .m_name = "cmark",
    .m_doc = "Utility functions for cmark and cmark-gfm API.",
    .m_size = -1,
  };

void cmark_gfm_core_extensions_ensure_registered(void);

PyMODINIT_FUNC
PyInit_cmark (void)
{
  PyObject *module;

  if (PyType_Ready (&CMarkType) < 0)
    return NULL;

  if ((module = PyModule_Create (&moduledef)) == NULL)
    return NULL;

  Py_INCREF (&CMarkType);
  if (PyModule_AddObject (module, "CMark", (PyObject *) &CMarkType) < 0)
    {
      Py_DECREF (&CMarkType);
      Py_DECREF (module);
      return NULL;
    }

  cmark_gfm_core_extensions_ensure_registered ();

  return module;
}
