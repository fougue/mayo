/****************************************************************************
** Meta object code from reading C++ file 'gui_document.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.12.8)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../src/gui/gui_document.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'gui_document.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.12.8. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_Mayo__GuiDocument_t {
    QByteArrayData data[14];
    char stringdata0[233];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_Mayo__GuiDocument_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_Mayo__GuiDocument_t qt_meta_stringdata_Mayo__GuiDocument = {
    {
QT_MOC_LITERAL(0, 0, 17), // "Mayo::GuiDocument"
QT_MOC_LITERAL(1, 18, 22), // "nodesVisibilityChanged"
QT_MOC_LITERAL(2, 41, 0), // ""
QT_MOC_LITERAL(3, 42, 45), // "std::unordered_map<TreeNodeId..."
QT_MOC_LITERAL(4, 88, 9), // "mapNodeId"
QT_MOC_LITERAL(5, 98, 26), // "graphicsBoundingBoxChanged"
QT_MOC_LITERAL(6, 125, 7), // "Bnd_Box"
QT_MOC_LITERAL(7, 133, 6), // "bndBox"
QT_MOC_LITERAL(8, 140, 24), // "viewTrihedronModeChanged"
QT_MOC_LITERAL(9, 165, 17), // "ViewTrihedronMode"
QT_MOC_LITERAL(10, 183, 4), // "mode"
QT_MOC_LITERAL(11, 188, 26), // "viewTrihedronCornerChanged"
QT_MOC_LITERAL(12, 215, 10), // "Qt::Corner"
QT_MOC_LITERAL(13, 226, 6) // "corner"

    },
    "Mayo::GuiDocument\0nodesVisibilityChanged\0"
    "\0std::unordered_map<TreeNodeId,Qt::CheckState>\0"
    "mapNodeId\0graphicsBoundingBoxChanged\0"
    "Bnd_Box\0bndBox\0viewTrihedronModeChanged\0"
    "ViewTrihedronMode\0mode\0"
    "viewTrihedronCornerChanged\0Qt::Corner\0"
    "corner"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_Mayo__GuiDocument[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       4,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   34,    2, 0x06 /* Public */,
       5,    1,   37,    2, 0x06 /* Public */,
       8,    1,   40,    2, 0x06 /* Public */,
      11,    1,   43,    2, 0x06 /* Public */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3,    4,
    QMetaType::Void, 0x80000000 | 6,    7,
    QMetaType::Void, 0x80000000 | 9,   10,
    QMetaType::Void, 0x80000000 | 12,   13,

       0        // eod
};

void Mayo::GuiDocument::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<GuiDocument *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->nodesVisibilityChanged((*reinterpret_cast< const std::unordered_map<TreeNodeId,Qt::CheckState>(*)>(_a[1]))); break;
        case 1: _t->graphicsBoundingBoxChanged((*reinterpret_cast< const Bnd_Box(*)>(_a[1]))); break;
        case 2: _t->viewTrihedronModeChanged((*reinterpret_cast< ViewTrihedronMode(*)>(_a[1]))); break;
        case 3: _t->viewTrihedronCornerChanged((*reinterpret_cast< Qt::Corner(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (GuiDocument::*)(const std::unordered_map<TreeNodeId,Qt::CheckState> & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&GuiDocument::nodesVisibilityChanged)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (GuiDocument::*)(const Bnd_Box & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&GuiDocument::graphicsBoundingBoxChanged)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (GuiDocument::*)(ViewTrihedronMode );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&GuiDocument::viewTrihedronModeChanged)) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (GuiDocument::*)(Qt::Corner );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&GuiDocument::viewTrihedronCornerChanged)) {
                *result = 3;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject Mayo::GuiDocument::staticMetaObject = { {
    &QObject::staticMetaObject,
    qt_meta_stringdata_Mayo__GuiDocument.data,
    qt_meta_data_Mayo__GuiDocument,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *Mayo::GuiDocument::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Mayo::GuiDocument::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_Mayo__GuiDocument.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int Mayo::GuiDocument::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 4)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 4)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 4;
    }
    return _id;
}

// SIGNAL 0
void Mayo::GuiDocument::nodesVisibilityChanged(const std::unordered_map<TreeNodeId,Qt::CheckState> & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void Mayo::GuiDocument::graphicsBoundingBoxChanged(const Bnd_Box & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void Mayo::GuiDocument::viewTrihedronModeChanged(ViewTrihedronMode _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void Mayo::GuiDocument::viewTrihedronCornerChanged(Qt::Corner _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
