/*
 * Copyright (c) 2013, Wesley Shields <wxs@atarininja.org>. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <Python.h>
#include <structmember.h>
#include "parse.h"

#define PEPY_VERSION "0.1"

/* These are used to across multiple objects. */
#define PEPY_OBJECT_GET(OBJ, ATTR) \
static PyObject *pepy_##OBJ##_get_##ATTR(PyObject *self, void *closure) { \
	Py_INCREF(((pepy_##OBJ *) self)->ATTR); \
	return ((pepy_##OBJ *) self)->ATTR; \
}

#define OBJECTGETTER(OBJ, ATTR, DOC) \
	{ (char *) #ATTR, (getter) pepy_##OBJ##_get_##ATTR, \
	  (setter) pepy_attr_not_writable, \
	  (char *) #DOC, NULL }

static PyObject *pepy_error;

typedef struct {
	PyObject_HEAD
} pepy;

typedef struct {
	PyObject_HEAD
	parsed_pe *pe;
} pepy_parsed;

typedef struct {
	PyObject_HEAD
	PyObject *name;
	PyObject *base;
	PyObject *length;
	PyObject *virtaddr;
	PyObject *virtsize;
	PyObject *numrelocs;
	PyObject *numlinenums;
	PyObject *characteristics;
} pepy_section;

typedef struct {
	PyObject_HEAD
	PyObject *name;
	PyObject *sym;
	PyObject *addr;
} pepy_import;

typedef struct {
	PyObject_HEAD
	PyObject *mod;
	PyObject *func;
	PyObject *addr;
} pepy_export;

typedef struct {
	PyObject_HEAD
	PyObject *type;
	PyObject *addr;
} pepy_relocation;

/* None of the attributes in these objects are writable. */
static int pepy_attr_not_writable(PyObject *self, PyObject *value, void *closure) {
	PyErr_SetString(PyExc_TypeError, "Attribute not writable");
	return -1;
}

static PyObject *pepy_import_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {
	pepy_import *self;

	self = (pepy_import *) type->tp_alloc(type, 0);

	return (PyObject *) self;
}

static int pepy_import_init(pepy_import *self, PyObject *args, PyObject *kwds) {
	if (!PyArg_ParseTuple(args, "OOO:pepy_import_init", &self->name, &self->sym, &self->addr))
		return -1;
	return 0;
}

static void pepy_import_dealloc(pepy_import *self) {
	Py_XDECREF(self->name);
	Py_XDECREF(self->sym);
	Py_XDECREF(self->addr);
	self->ob_type->tp_free((PyObject *) self);
}

PEPY_OBJECT_GET(import, name)
PEPY_OBJECT_GET(import, sym)
PEPY_OBJECT_GET(import, addr)

static PyGetSetDef pepy_import_getseters[] = {
	OBJECTGETTER(import, name, "Name"),
	OBJECTGETTER(import, sym, "Symbol"),
	OBJECTGETTER(import, addr, "Address"),
	{ NULL }
};

static PyTypeObject pepy_import_type = {
	PyObject_HEAD_INIT(NULL)
	0,                                /* ob_size */
	"pepy.import",                    /* tp_name */
	sizeof(pepy_import),              /* tp_basicsize */
	0,                                /* tp_itemsize */
	(destructor) pepy_import_dealloc, /* tp_dealloc */
	0,                                /* tp_print */
	0,                                /* tp_getattr */
	0,                                /* tp_setattr */
	0,                                /* tp_compare */
	0,                                /* tp_repr */
	0,                                /* tp_as_number */
	0,                                /* tp_as_sequence */
	0,                                /* tp_as_mapping */
	0,                                /* tp_hash */
	0,                                /* tp_call */
	0,                                /* tp_str */
	0,                                /* tp_getattro */
	0,                                /* tp_setattro */
	0,                                /* tp_as_buffer */
	Py_TPFLAGS_DEFAULT,               /* tp_flags */
	"pepy import object",             /* tp_doc */
	0,                                /* tp_traverse */
	0,                                /* tp_clear */
	0,                                /* tp_richcompare */
	0,                                /* tp_weaklistoffset */
	0,                                /* tp_iter */
	0,                                /* tp_iternext */
	0,                                /* tp_methods */
	0,                                /* tp_members */
	pepy_import_getseters,            /* tp_getset */
	0,                                /* tp_base */
	0,                                /* tp_dict */
	0,                                /* tp_descr_get */
	0,                                /* tp_descr_set */
	0,                                /* tp_dictoffset */
	(initproc) pepy_import_init,      /* tp_init */
	0,                                /* tp_alloc */
	pepy_import_new                   /* tp_new */
};

static PyObject *pepy_export_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {
	pepy_export *self;

	self = (pepy_export *) type->tp_alloc(type, 0);

	return (PyObject *) self;
}

static int pepy_export_init(pepy_export *self, PyObject *args, PyObject *kwds) {
	if (!PyArg_ParseTuple(args, "OOO:pepy_export_init", &self->mod, &self->func, &self->addr))
		return -1;
	return 0;
}

static void pepy_export_dealloc(pepy_export *self) {
	Py_XDECREF(self->mod);
	Py_XDECREF(self->func);
	Py_XDECREF(self->addr);
	self->ob_type->tp_free((PyObject *) self);
}

PEPY_OBJECT_GET(export, mod)
PEPY_OBJECT_GET(export, func)
PEPY_OBJECT_GET(export, addr)

static PyGetSetDef pepy_export_getseters[] = {
	OBJECTGETTER(export, mod, "Module"),
	OBJECTGETTER(export, func, "Function"),
	OBJECTGETTER(export, addr, "Address"),
	{ NULL }
};

static PyTypeObject pepy_export_type = {
	PyObject_HEAD_INIT(NULL)
	0,                                /* ob_size */
	"pepy.export",                    /* tp_name */
	sizeof(pepy_export),              /* tp_basicsize */
	0,                                /* tp_itemsize */
	(destructor) pepy_export_dealloc, /* tp_dealloc */
	0,                                /* tp_print */
	0,                                /* tp_getattr */
	0,                                /* tp_setattr */
	0,                                /* tp_compare */
	0,                                /* tp_repr */
	0,                                /* tp_as_number */
	0,                                /* tp_as_sequence */
	0,                                /* tp_as_mapping */
	0,                                /* tp_hash */
	0,                                /* tp_call */
	0,                                /* tp_str */
	0,                                /* tp_getattro */
	0,                                /* tp_setattro */
	0,                                /* tp_as_buffer */
	Py_TPFLAGS_DEFAULT,               /* tp_flags */
	"pepy export object",             /* tp_doc */
	0,                                /* tp_traverse */
	0,                                /* tp_clear */
	0,                                /* tp_richcompare */
	0,                                /* tp_weaklistoffset */
	0,                                /* tp_iter */
	0,                                /* tp_iternext */
	0,                                /* tp_methods */
	0,                                /* tp_members */
	pepy_export_getseters,            /* tp_getset */
	0,                                /* tp_base */
	0,                                /* tp_dict */
	0,                                /* tp_descr_get */
	0,                                /* tp_descr_set */
	0,                                /* tp_dictoffset */
	(initproc) pepy_export_init,      /* tp_init */
	0,                                /* tp_alloc */
	pepy_export_new                   /* tp_new */
};

static PyObject *pepy_relocation_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {
	pepy_relocation *self;

	self = (pepy_relocation *) type->tp_alloc(type, 0);

	return (PyObject *) self;
}

static int pepy_relocation_init(pepy_relocation *self, PyObject *args, PyObject *kwds) {
	if (!PyArg_ParseTuple(args, "OO:pepy_relocation_init", &self->type, &self->addr))
		return -1;
	return 0;
}

static void pepy_relocation_dealloc(pepy_relocation *self) {
	Py_XDECREF(self->type);
	Py_XDECREF(self->addr);
	self->ob_type->tp_free((PyObject *) self);
}

PEPY_OBJECT_GET(relocation, type)
PEPY_OBJECT_GET(relocation, addr)

static PyGetSetDef pepy_relocation_getseters[] = {
	OBJECTGETTER(relocation, type, "Type"),
	OBJECTGETTER(relocation, addr, "Address"),
	{ NULL }
};

static PyTypeObject pepy_relocation_type = {
	PyObject_HEAD_INIT(NULL)
	0,                                    /* ob_size */
	"pepy.relocation",                    /* tp_name */
	sizeof(pepy_relocation),              /* tp_basicsize */
	0,                                    /* tp_itemsize */
	(destructor) pepy_relocation_dealloc, /* tp_dealloc */
	0,                                    /* tp_print */
	0,                                    /* tp_getattr */
	0,                                    /* tp_setattr */
	0,                                    /* tp_compare */
	0,                                    /* tp_repr */
	0,                                    /* tp_as_number */
	0,                                    /* tp_as_sequence */
	0,                                    /* tp_as_mapping */
	0,                                    /* tp_hash */
	0,                                    /* tp_call */
	0,                                    /* tp_str */
	0,                                    /* tp_getattro */
	0,                                    /* tp_setattro */
	0,                                    /* tp_as_buffer */
	Py_TPFLAGS_DEFAULT,                   /* tp_flags */
	"pepy relocation object",             /* tp_doc */
	0,                                    /* tp_traverse */
	0,                                    /* tp_clear */
	0,                                    /* tp_richcompare */
	0,                                    /* tp_weaklistoffset */
	0,                                    /* tp_iter */
	0,                                    /* tp_iternext */
	0,                                    /* tp_methods */
	0,                                    /* tp_members */
	pepy_relocation_getseters,            /* tp_getset */
	0,                                    /* tp_base */
	0,                                    /* tp_dict */
	0,                                    /* tp_descr_get */
	0,                                    /* tp_descr_set */
	0,                                    /* tp_dictoffset */
	(initproc) pepy_relocation_init,      /* tp_init */
	0,                                    /* tp_alloc */
	pepy_relocation_new                   /* tp_new */
};

static PyObject *pepy_section_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {
	pepy_section *self;

	self = (pepy_section *) type->tp_alloc(type, 0);

	return (PyObject *) self;
}

static int pepy_section_init(pepy_section *self, PyObject *args, PyObject *kwds) {
	if (!PyArg_ParseTuple(args, "OOOOOOOO:pepy_section_init", &self->name, &self->base, &self->length, &self->virtaddr, &self->virtsize, &self->numrelocs, &self->numlinenums, &self->characteristics))
		return -1;
	return 0;
}

static void pepy_section_dealloc(pepy_section *self) {
	Py_XDECREF(self->name);
	Py_XDECREF(self->base);
	Py_XDECREF(self->length);
	Py_XDECREF(self->virtaddr);
	Py_XDECREF(self->virtsize);
	Py_XDECREF(self->numrelocs);
	Py_XDECREF(self->numlinenums);
	Py_XDECREF(self->characteristics);
	self->ob_type->tp_free((PyObject *) self);
}

PEPY_OBJECT_GET(section, name)
PEPY_OBJECT_GET(section, base)
PEPY_OBJECT_GET(section, length)
PEPY_OBJECT_GET(section, virtaddr)
PEPY_OBJECT_GET(section, virtsize)
PEPY_OBJECT_GET(section, numrelocs)
PEPY_OBJECT_GET(section, numlinenums)
PEPY_OBJECT_GET(section, characteristics)

static PyGetSetDef pepy_section_getseters[] = {
	OBJECTGETTER(section, name, "Name"),
	OBJECTGETTER(section, base, "Base address"),
	OBJECTGETTER(section, length, "Length"),
	OBJECTGETTER(section, virtaddr, "Virtual address"),
	OBJECTGETTER(section, virtsize, "Virtual size"),
	OBJECTGETTER(section, numrelocs, "Number of relocations"),
	OBJECTGETTER(section, numlinenums, "Number of line numbers"),
	OBJECTGETTER(section, characteristics, "Characteristics"),
	{ NULL }
};

static PyTypeObject pepy_section_type = {
	PyObject_HEAD_INIT(NULL)
	0,                                 /* ob_size */
	"pepy.section",                    /* tp_name */
	sizeof(pepy_section),              /* tp_basicsize */
	0,                                 /* tp_itemsize */
	(destructor) pepy_section_dealloc, /* tp_dealloc */
	0,                                 /* tp_print */
	0,                                 /* tp_getattr */
	0,                                 /* tp_setattr */
	0,                                 /* tp_compare */
	0,                                 /* tp_repr */
	0,                                 /* tp_as_number */
	0,                                 /* tp_as_sequence */
	0,                                 /* tp_as_mapping */
	0,                                 /* tp_hash */
	0,                                 /* tp_call */
	0,                                 /* tp_str */
	0,                                 /* tp_getattro */
	0,                                 /* tp_setattro */
	0,                                 /* tp_as_buffer */
	Py_TPFLAGS_DEFAULT,                /* tp_flags */
	"pepy section object",             /* tp_doc */
	0,                                 /* tp_traverse */
	0,                                 /* tp_clear */
	0,                                 /* tp_richcompare */
	0,                                 /* tp_weaklistoffset */
	0,                                 /* tp_iter */
	0,                                 /* tp_iternext */
	0,                                 /* tp_methods */
	0,                                 /* tp_members */
	pepy_section_getseters,            /* tp_getset */
	0,                                 /* tp_base */
	0,                                 /* tp_dict */
	0,                                 /* tp_descr_get */
	0,                                 /* tp_descr_set */
	0,                                 /* tp_dictoffset */
	(initproc) pepy_section_init,      /* tp_init */
	0,                                 /* tp_alloc */
	pepy_section_new                   /* tp_new */
};

static PyObject *pepy_parsed_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {
	pepy_parsed *self;

	self = (pepy_parsed *) type->tp_alloc(type, 0);

	return (PyObject *) self;
}

static int pepy_parsed_init(pepy_parsed *self, PyObject *args, PyObject *kwds) {
	char *pe_path;

	if (!PyArg_ParseTuple(args, "s:pepy_parse", &pe_path))
		return -1;

	if (!pe_path)
		return -1;

	self->pe = ParsePEFromFile(pe_path);
	if (!self->pe) {
		return -2;
	}

	return 0;
}

static void pepy_parsed_dealloc(pepy_parsed *self) {
	DestructParsedPE(self->pe);
	self->ob_type->tp_free((PyObject *) self);
}

static PyObject *pepy_parsed_get_entry_point(PyObject *self, PyObject *args) {
	VA entrypoint;
	PyObject *ret;

	if (!GetEntryPoint(((pepy_parsed *) self)->pe, entrypoint))
		Py_RETURN_NONE;

	ret = PyLong_FromLongLong(entrypoint);
	if (!ret) {
		PyErr_SetString(pepy_error, "Unable to create return object.");
		return NULL;
	}

	return ret;
}

static PyObject *pepy_parsed_get_bytes(PyObject *self, PyObject *args) {
	uint64_t start, idx;
	uint8_t b;
	Py_ssize_t len;
	PyObject *byte, *tmp, *ret;

	if (!PyArg_ParseTuple(args, "KK:pepy_parsed_get_bytes", &start, &len))
		return NULL;

	/*
	 * XXX: I don't think this is the best way to do this. I want a
	 * ByteArray object to be returned so first put each byte in a
	 * list and then call PyByteArray_FromObject to get the byte array.
	 */
	tmp = PyList_New(len);
	if (!tmp) {
		PyErr_SetString(pepy_error, "Unable to create new list.");
		return NULL;
	}

	for (idx = 0; idx < len; idx++) {
		if (!ReadByteAtVA(((pepy_parsed *) self)->pe, start + idx, b))
			break;

		byte = PyInt_FromLong(b);
		PyList_SET_ITEM(tmp, idx, byte);
		Py_DECREF(byte);
	}

	ret = PyByteArray_FromObject(tmp);
	if (!ret) {
		PyErr_SetString(pepy_error, "Unable to create new list.");
		return NULL;
	}
	Py_DECREF(tmp);

	return ret;
}

int section_callback(void *cbd, VA base, std::string &name, image_section_header s, bounded_buffer *data) {
	PyObject *sect;
	PyObject *tuple;
	PyObject *list = (PyObject *) cbd;

	/*
	 * The tuple item order is important here. It is passed into the
	 * section type initialization and parsed there.
	 */
	tuple = Py_BuildValue("sKKIIHHI", name.c_str(), base, data->bufLen,
	                      s.VirtualAddress, s.Misc.VirtualSize,
	                      s.NumberOfRelocations, s.NumberOfLinenumbers,
	                      s.Characteristics);
	if (!tuple)
		return 1;

	sect = pepy_section_new(&pepy_section_type, NULL, NULL);
	if (!sect) {
		Py_DECREF(tuple);
		return 1;
	}

	if (pepy_section_init((pepy_section *) sect, tuple, NULL) == -1) {
		PyErr_SetString(pepy_error, "Unable to init new section");
		return 1;
	}

	if (PyList_Append(list, sect) == -1) {
		Py_DECREF(tuple);
		Py_DECREF(sect);
		return 1;
	}

	return 0;
}

static PyObject *pepy_parsed_get_sections(PyObject *self, PyObject *args) {
	PyObject *ret = PyList_New(0);
	if (!ret) {
		PyErr_SetString(pepy_error, "Unable to create new list.");
		return NULL;
	}

	IterSec(((pepy_parsed *) self)->pe, section_callback, ret);

	return ret;
}

int import_callback(void *cbd, VA addr, std::string &name, std::string &sym) {
	PyObject *imp;
	PyObject *tuple;
	PyObject *list = (PyObject *) cbd;

	/*
	 * The tuple item order is important here. It is passed into the
	 * import type initialization and parsed there.
	 */
	tuple = Py_BuildValue("ssI", name.c_str(), sym.c_str(), addr);
	if (!tuple)
		return 1;

	imp = pepy_import_new(&pepy_import_type, NULL, NULL);
	if (!imp) {
		Py_DECREF(tuple);
		return 1;
	}

	if (pepy_import_init((pepy_import *) imp, tuple, NULL) == -1) {
		PyErr_SetString(pepy_error, "Unable to init new section");
		return 1;
	}

	if (PyList_Append(list, imp) == -1) {
		Py_DECREF(tuple);
		Py_DECREF(imp);
		return 1;
	}

	return 0;
}

static PyObject *pepy_parsed_get_imports(PyObject *self, PyObject *args) {
	PyObject *ret = PyList_New(0);
	if (!ret) {
		PyErr_SetString(pepy_error, "Unable to create new list.");
		return NULL;
	}

	IterImpVAString(((pepy_parsed *) self)->pe, import_callback, ret);

	return ret;
}

int export_callback(void *cbd, VA addr, std::string &mod, std::string &func) {
	PyObject *exp;
	PyObject *tuple;
	PyObject *list = (PyObject *) cbd;

	/*
	 * The tuple item order is important here. It is passed into the
	 * export type initialization and parsed there.
	 */
	tuple = Py_BuildValue("ssI", mod.c_str(), func.c_str(), addr);
	if (!tuple)
		return 1;

	exp = pepy_export_new(&pepy_export_type, NULL, NULL);
	if (!exp) {
		Py_DECREF(tuple);
		return 1;
	}

	if (pepy_export_init((pepy_export *) exp, tuple, NULL) == -1) {
		PyErr_SetString(pepy_error, "Unable to init new section");
		return 1;
	}

	if (PyList_Append(list, exp) == -1) {
		Py_DECREF(tuple);
		Py_DECREF(exp);
		return 1;
	}

	return 0;
}

static PyObject *pepy_parsed_get_exports(PyObject *self, PyObject *args) {
	PyObject *ret = PyList_New(0);
	if (!ret) {
		PyErr_SetString(pepy_error, "Unable to create new list.");
		return NULL;
	}

	/*
	 * This could use the same callback and object as imports but the names
	 * of the attributes would be slightly off.
	 */
	IterExpVA(((pepy_parsed *) self)->pe, export_callback, ret);

	return ret;
}

int reloc_callback(void *cbd, VA addr, reloc_type type) {
	PyObject *reloc;
	PyObject *tuple;
	PyObject *list = (PyObject *) cbd;

	/*
	 * The tuple item order is important here. It is passed into the
	 * relocation type initialization and parsed there.
	 */
	tuple = Py_BuildValue("II", type, addr);
	if (!tuple)
		return 1;

	reloc = pepy_relocation_new(&pepy_relocation_type, NULL, NULL);
	if (!reloc) {
		Py_DECREF(tuple);
		return 1;
	}

	if (pepy_relocation_init((pepy_relocation *) reloc, tuple, NULL) == -1) {
		PyErr_SetString(pepy_error, "Unable to init new section");
		return 1;
	}

	if (PyList_Append(list, reloc) == -1) {
		Py_DECREF(tuple);
		Py_DECREF(reloc);
		return 1;
	}

	return 0;
}

static PyObject *pepy_parsed_get_relocations(PyObject *self, PyObject *args) {
	PyObject *ret = PyList_New(0);
	if (!ret) {
		PyErr_SetString(pepy_error, "Unable to create new list.");
		return NULL;
	}

	IterRelocs(((pepy_parsed *) self)->pe, reloc_callback, ret);

	return ret;
}

#define PEPY_PARSED_GET(ATTR, VAL) \
static PyObject *pepy_parsed_get_##ATTR(PyObject *self, void *closure) { \
	PyObject *ret = PyInt_FromLong(((pepy_parsed *) self)->pe->peHeader.VAL); \
	if (!ret) \
		PyErr_SetString(PyExc_AttributeError, "Error getting attribute"); \
	return ret; \
}

PEPY_PARSED_GET(signature, nt.Signature)
PEPY_PARSED_GET(machine, nt.FileHeader.Machine)
PEPY_PARSED_GET(numberofsections, nt.FileHeader.NumberOfSections)
PEPY_PARSED_GET(timedatestamp, nt.FileHeader.TimeDateStamp)
PEPY_PARSED_GET(numberofsymbols, nt.FileHeader.NumberOfSymbols)
PEPY_PARSED_GET(characteristics, nt.FileHeader.Characteristics)
PEPY_PARSED_GET(magic, nt.OptionalHeader.Magic)
PEPY_PARSED_GET(majorlinkerver, nt.OptionalHeader.MajorLinkerVersion)
PEPY_PARSED_GET(minorlinkerver, nt.OptionalHeader.MinorLinkerVersion)
PEPY_PARSED_GET(codesize, nt.OptionalHeader.SizeOfCode);
PEPY_PARSED_GET(initdatasize, nt.OptionalHeader.SizeOfInitializedData);
PEPY_PARSED_GET(uninitdatasize, nt.OptionalHeader.SizeOfUninitializedData);
PEPY_PARSED_GET(entrypointaddr, nt.OptionalHeader.AddressOfEntryPoint);
PEPY_PARSED_GET(baseofcode, nt.OptionalHeader.BaseOfCode);
PEPY_PARSED_GET(baseofdata, nt.OptionalHeader.BaseOfData);
PEPY_PARSED_GET(imagebase, nt.OptionalHeader.ImageBase);
PEPY_PARSED_GET(sectionalignement, nt.OptionalHeader.SectionAlignment);
PEPY_PARSED_GET(filealingment, nt.OptionalHeader.FileAlignment);
PEPY_PARSED_GET(majorosver, nt.OptionalHeader.MajorOperatingSystemVersion);
PEPY_PARSED_GET(minorosver, nt.OptionalHeader.MinorOperatingSystemVersion);
PEPY_PARSED_GET(win32ver, nt.OptionalHeader.Win32VersionValue);
PEPY_PARSED_GET(imagesize, nt.OptionalHeader.SizeOfImage);
PEPY_PARSED_GET(headersize, nt.OptionalHeader.SizeOfHeaders);
PEPY_PARSED_GET(checksum, nt.OptionalHeader.CheckSum);
PEPY_PARSED_GET(subsystem, nt.OptionalHeader.Subsystem);
PEPY_PARSED_GET(dllcharacteristics, nt.OptionalHeader.DllCharacteristics);
PEPY_PARSED_GET(stackreservesize, nt.OptionalHeader.SizeOfStackReserve);
PEPY_PARSED_GET(stackcommitsize, nt.OptionalHeader.SizeOfStackCommit);
PEPY_PARSED_GET(heapreservesize, nt.OptionalHeader.SizeOfHeapReserve);
PEPY_PARSED_GET(heapcommitsize, nt.OptionalHeader.SizeOfHeapCommit);
PEPY_PARSED_GET(loaderflags, nt.OptionalHeader.LoaderFlags);
PEPY_PARSED_GET(rvasandsize, nt.OptionalHeader.NumberOfRvaAndSizes);

static PyGetSetDef pepy_parsed_getseters[] = {
	OBJECTGETTER(parsed, signature, "PE Signature"),
	OBJECTGETTER(parsed, machine, "Machine"),
	OBJECTGETTER(parsed, numberofsections, "Number of sections"),
	OBJECTGETTER(parsed, timedatestamp, "Timedate stamp"),
	OBJECTGETTER(parsed, numberofsymbols, "Number of symbols"),
	OBJECTGETTER(parsed, characteristics, "Characteristics"),
	OBJECTGETTER(parsed, magic, "Magic"),
	OBJECTGETTER(parsed, majorlinkerver, "Major linker version"),
	OBJECTGETTER(parsed, minorlinkerver, "Minor linker version"),
	OBJECTGETTER(parsed, codesize, "Size of code"),
	OBJECTGETTER(parsed, initdatasize, "Size of initialized data"),
	OBJECTGETTER(parsed, uninitdatasize, "Size of uninitialized data"),
	OBJECTGETTER(parsed, entrypointaddr, "Address of entry point"),
	OBJECTGETTER(parsed, baseofcode, "Base address of code"),
	OBJECTGETTER(parsed, baseofdata, "Base address of data"),
	OBJECTGETTER(parsed, imagebase, "Image base address"),
	OBJECTGETTER(parsed, sectionalignement, "Section alignment"),
	OBJECTGETTER(parsed, filealingment, "File alignment"),
	OBJECTGETTER(parsed, majorosver, "Major OS version"),
	OBJECTGETTER(parsed, minorosver, "Minor OS version"),
	OBJECTGETTER(parsed, win32ver, "Win32 version"),
	OBJECTGETTER(parsed, imagesize, "Size of image"),
	OBJECTGETTER(parsed, headersize, "Size of headers"),
	OBJECTGETTER(parsed, checksum, "Checksum"),
	OBJECTGETTER(parsed, subsystem, "Subsystem"),
	OBJECTGETTER(parsed, dllcharacteristics, "DLL characteristics"),
	OBJECTGETTER(parsed, stackreservesize, "Size of stack reserve"),
	OBJECTGETTER(parsed, stackcommitsize, "Size of stack commit"),
	OBJECTGETTER(parsed, heapreservesize, "Size of heap reserve"),
	OBJECTGETTER(parsed, heapcommitsize, "Size of heap commit"),
	OBJECTGETTER(parsed, loaderflags, "Loader flags"),
	OBJECTGETTER(parsed, rvasandsize, "Number of RVA and sizes"),
	{ NULL }
};

static PyMethodDef pepy_parsed_methods[] = {
	{ "get_entry_point", pepy_parsed_get_entry_point, METH_NOARGS,
	  "Return the entry point address." },
	{ "get_bytes", pepy_parsed_get_bytes, METH_VARARGS,
	  "Return the first N bytes at a given address." },
	{ "get_sections", pepy_parsed_get_sections, METH_NOARGS,
	  "Return a list of section objects." },
	{ "get_imports", pepy_parsed_get_imports, METH_NOARGS,
	  "Return a list of import objects." },
	{ "get_exports", pepy_parsed_get_exports, METH_NOARGS,
	  "Return a list of export objects." },
	{ "get_relocations", pepy_parsed_get_relocations, METH_NOARGS,
	  "Return a list of relocation objects." },
	{ NULL }
};

static PyTypeObject pepy_parsed_type = {
	PyObject_HEAD_INIT(NULL)
	0,                                         /* ob_size */
	"pepy.parsed",                             /* tp_name */
	sizeof(pepy_parsed),                       /* tp_basicsize */
	0,                                         /* tp_itemsize */
	(destructor) pepy_parsed_dealloc,          /* tp_dealloc */
	0,                                         /* tp_print */
	0,                                         /* tp_getattr */
	0,                                         /* tp_setattr */
	0,                                         /* tp_compare */
	0,                                         /* tp_repr */
	0,                                         /* tp_as_number */
	0,                                         /* tp_as_sequence */
	0,                                         /* tp_as_mapping */
	0,                                         /* tp_hash */
	0,                                         /* tp_call */
	0,                                         /* tp_str */
	0,                                         /* tp_getattro */
	0,                                         /* tp_setattro */
	0,                                         /* tp_as_buffer */
	Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /* tp_flags */
	"pepy parsed object",                      /* tp_doc */
	0,                                         /* tp_traverse */
	0,                                         /* tp_clear */
	0,                                         /* tp_richcompare */
	0,                                         /* tp_weaklistoffset */
	0,                                         /* tp_iter */
	0,                                         /* tp_iternext */
	pepy_parsed_methods,                       /* tp_methods */
	0,                                         /* tp_members */
	pepy_parsed_getseters,                     /* tp_getset */
	0,                                         /* tp_base */
	0,                                         /* tp_dict */
	0,                                         /* tp_descr_get */
	0,                                         /* tp_descr_set */
	0,                                         /* tp_dictoffset */
	(initproc) pepy_parsed_init,               /* tp_init */
	0,                                         /* tp_alloc */
	pepy_parsed_new                            /* tp_new */
};

static PyObject *pepy_parse(PyObject *self, PyObject *args) {
	PyObject *parsed;
	int ret;

	parsed = pepy_parsed_new(&pepy_parsed_type, NULL, NULL);
	if (!parsed) {
		PyErr_SetString(pepy_error, "Unable to make new parsed object.");
		return NULL;
	}

	ret = pepy_parsed_init((pepy_parsed *) parsed, args, NULL);
	if (ret < 0) {
		if (ret == -2)
			PyErr_SetString(pepy_error, "Unable to parse PE file.");
		else
			PyErr_SetString(pepy_error, "Unable to init new parsed object.");
		return NULL;
	}

	return parsed;
}

static PyMethodDef pepy_methods[] = {
	{ "parse", pepy_parse, METH_VARARGS, "Parse PE from file." },
	{ NULL }
};

PyMODINIT_FUNC initpepy(void) {
	PyObject *m;

	if (PyType_Ready(&pepy_parsed_type) < 0 ||
	    PyType_Ready(&pepy_section_type) < 0 ||
	    PyType_Ready(&pepy_import_type) < 0 ||
	    PyType_Ready(&pepy_export_type) < 0 ||
	    PyType_Ready(&pepy_relocation_type) < 0)
		return;

	m = Py_InitModule3("pepy", pepy_methods, "Python interface to pe-parse.");
	if (!m)
		return;

	pepy_error = PyErr_NewException((char *) "pepy.error", NULL, NULL);
	Py_INCREF(pepy_error);
	PyModule_AddObject(m, "error", pepy_error);

	Py_INCREF(&pepy_parsed_type);
	PyModule_AddObject(m, "pepy_parsed", (PyObject *) &pepy_parsed_type);

	Py_INCREF(&pepy_section_type);
	PyModule_AddObject(m, "pepy_section", (PyObject *) &pepy_section_type);

	Py_INCREF(&pepy_import_type);
	PyModule_AddObject(m, "pepy_import", (PyObject *) &pepy_import_type);

	Py_INCREF(&pepy_export_type);
	PyModule_AddObject(m, "pepy_export", (PyObject *) &pepy_export_type);

	Py_INCREF(&pepy_relocation_type);
	PyModule_AddObject(m, "pepy_relocation", (PyObject *) &pepy_relocation_type);

	PyModule_AddStringMacro(m, PEPY_VERSION);

	PyModule_AddIntMacro(m, MZ_MAGIC);
	PyModule_AddIntMacro(m, NT_MAGIC);
	PyModule_AddIntMacro(m, NUM_DIR_ENTRIES);
	PyModule_AddIntMacro(m, NT_OPTIONAL_32_MAGIC);
	PyModule_AddIntMacro(m, NT_SHORT_NAME_LEN);
	PyModule_AddIntMacro(m, DIR_EXPORT);
	PyModule_AddIntMacro(m, DIR_IMPORT);
	PyModule_AddIntMacro(m, DIR_RESOURCE);
	PyModule_AddIntMacro(m, DIR_EXCEPTION);
	PyModule_AddIntMacro(m, DIR_SECURITY);
	PyModule_AddIntMacro(m, DIR_BASERELOC);
	PyModule_AddIntMacro(m, DIR_DEBUG);
	PyModule_AddIntMacro(m, DIR_ARCHITECTURE);
	PyModule_AddIntMacro(m, DIR_GLOBALPTR);
	PyModule_AddIntMacro(m, DIR_TLS);
	PyModule_AddIntMacro(m, DIR_LOAD_CONFIG);
	PyModule_AddIntMacro(m, DIR_BOUND_IMPORT);
	PyModule_AddIntMacro(m, DIR_IAT);
	PyModule_AddIntMacro(m, DIR_DELAY_IMPORT);
	PyModule_AddIntMacro(m, DIR_COM_DESCRIPTOR);

	PyModule_AddIntMacro(m, IMAGE_SCN_TYPE_NO_PAD);
	PyModule_AddIntMacro(m, IMAGE_SCN_CNT_CODE);
	PyModule_AddIntMacro(m, IMAGE_SCN_CNT_INITIALIZED_DATA);
	PyModule_AddIntMacro(m, IMAGE_SCN_CNT_UNINITIALIZED_DATA);
	PyModule_AddIntMacro(m, IMAGE_SCN_LNK_OTHER);
	PyModule_AddIntMacro(m, IMAGE_SCN_LNK_INFO);
	PyModule_AddIntMacro(m, IMAGE_SCN_LNK_REMOVE);
	PyModule_AddIntMacro(m, IMAGE_SCN_LNK_COMDAT);
	PyModule_AddIntMacro(m, IMAGE_SCN_NO_DEFER_SPEC_EXC);
	PyModule_AddIntMacro(m, IMAGE_SCN_GPREL);
	PyModule_AddIntMacro(m, IMAGE_SCN_MEM_FARDATA);
	PyModule_AddIntMacro(m, IMAGE_SCN_MEM_PURGEABLE);
	PyModule_AddIntMacro(m, IMAGE_SCN_MEM_16BIT);
	PyModule_AddIntMacro(m, IMAGE_SCN_MEM_LOCKED);
	PyModule_AddIntMacro(m, IMAGE_SCN_MEM_PRELOAD);
	PyModule_AddIntMacro(m, IMAGE_SCN_ALIGN_1BYTES);
	PyModule_AddIntMacro(m, IMAGE_SCN_ALIGN_2BYTES);
	PyModule_AddIntMacro(m, IMAGE_SCN_ALIGN_4BYTES);
	PyModule_AddIntMacro(m, IMAGE_SCN_ALIGN_8BYTES);
	PyModule_AddIntMacro(m, IMAGE_SCN_ALIGN_16BYTES);
	PyModule_AddIntMacro(m, IMAGE_SCN_ALIGN_32BYTES);
	PyModule_AddIntMacro(m, IMAGE_SCN_ALIGN_64BYTES);
	PyModule_AddIntMacro(m, IMAGE_SCN_ALIGN_128BYTES);
	PyModule_AddIntMacro(m, IMAGE_SCN_ALIGN_256BYTES);
	PyModule_AddIntMacro(m, IMAGE_SCN_ALIGN_512BYTES);
	PyModule_AddIntMacro(m, IMAGE_SCN_ALIGN_1024BYTES);
	PyModule_AddIntMacro(m, IMAGE_SCN_ALIGN_2048BYTES);
	PyModule_AddIntMacro(m, IMAGE_SCN_ALIGN_4096BYTES);
	PyModule_AddIntMacro(m, IMAGE_SCN_ALIGN_8192BYTES);
	PyModule_AddIntMacro(m, IMAGE_SCN_ALIGN_MASK);
	PyModule_AddIntMacro(m, IMAGE_SCN_LNK_NRELOC_OVFL);
	PyModule_AddIntMacro(m, IMAGE_SCN_MEM_DISCARDABLE);
	PyModule_AddIntMacro(m, IMAGE_SCN_MEM_NOT_CACHED);
	PyModule_AddIntMacro(m, IMAGE_SCN_MEM_NOT_PAGED);
	PyModule_AddIntMacro(m, IMAGE_SCN_MEM_SHARED);
	PyModule_AddIntMacro(m, IMAGE_SCN_MEM_EXECUTE);
	PyModule_AddIntMacro(m, IMAGE_SCN_MEM_READ);
	PyModule_AddIntMacro(m, IMAGE_SCN_MEM_WRITE);
}
