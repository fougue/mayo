/****************************************************************************
** Meta object code from reading C++ file 'application.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.12.8)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../src/base/application.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'application.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.12.8. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_Mayo__Application_t {
    QByteArrayData data[12];
    char stringdata0[180];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_Mayo__Application_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_Mayo__Application_t qt_meta_stringdata_Mayo__Application = {
    {
QT_MOC_LITERAL(0, 0, 17), // "Mayo::Application"
QT_MOC_LITERAL(1, 18, 13), // "documentAdded"
QT_MOC_LITERAL(2, 32, 0), // ""
QT_MOC_LITERAL(3, 33, 17), // "Mayo::DocumentPtr"
QT_MOC_LITERAL(4, 51, 3), // "doc"
QT_MOC_LITERAL(5, 55, 20), // "documentAboutToClose"
QT_MOC_LITERAL(6, 76, 19), // "documentNameChanged"
QT_MOC_LITERAL(7, 96, 4), // "name"
QT_MOC_LITERAL(8, 101, 19), // "documentEntityAdded"
QT_MOC_LITERAL(9, 121, 16), // "Mayo::TreeNodeId"
QT_MOC_LITERAL(10, 138, 8), // "entityId"
QT_MOC_LITERAL(11, 147, 32) // "documentEntityAboutToBeDestroyed"

    },
    "Mayo::Application\0documentAdded\0\0"
    "Mayo::DocumentPtr\0doc\0documentAboutToClose\0"
    "documentNameChanged\0name\0documentEntityAdded\0"
    "Mayo::TreeNodeId\0entityId\0"
    "documentEntityAboutToBeDestroyed"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_Mayo__Application[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       5,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   39,    2, 0x06 /* Public */,
       5,    1,   42,    2, 0x06 /* Public */,
       6,    2,   45,    2, 0x06 /* Public */,
       8,    2,   50,    2, 0x06 /* Public */,
      11,    2,   55,    2, 0x06 /* Public */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3,    4,
    QMetaType::Void, 0x80000000 | 3,    4,
    QMetaType::Void, 0x80000000 | 3, QMetaType::QString,    4,    7,
    QMetaType::Void, 0x80000000 | 3, 0x80000000 | 9,    4,   10,
    QMetaType::Void, 0x80000000 | 3, 0x80000000 | 9,    4,   10,

       0        // eod
};

void Mayo::Application::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<Application *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->documentAdded((*reinterpret_cast< const Mayo::DocumentPtr(*)>(_a[1]))); break;
        case 1: _t->documentAboutToClose((*reinterpret_cast< const Mayo::DocumentPtr(*)>(_a[1]))); break;
        case 2: _t->documentNameChanged((*reinterpret_cast< const Mayo::DocumentPtr(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2]))); break;
        case 3: _t->documentEntityAdded((*reinterpret_cast< const Mayo::DocumentPtr(*)>(_a[1])),(*reinterpret_cast< Mayo::TreeNodeId(*)>(_a[2]))); break;
        case 4: _t->documentEntityAboutToBeDestroyed((*reinterpret_cast< const Mayo::DocumentPtr(*)>(_a[1])),(*reinterpret_cast< Mayo::TreeNodeId(*)>(_a[2]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (Application::*)(const Mayo::DocumentPtr & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&Application::documentAdded)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (Application::*)(const Mayo::DocumentPtr & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&Application::documentAboutToClose)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (Application::*)(const Mayo::DocumentPtr & , const QString & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&Application::documentNameChanged)) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (Application::*)(const Mayo::DocumentPtr & , Mayo::TreeNodeId );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&Application::documentEntityAdded)) {
                *result = 3;
                return;
            }
        }
        {
            using _t = void (Application::*)(const Mayo::DocumentPtr & , Mayo::TreeNodeId );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&Application::documentEntityAboutToBeDestroyed)) {
                *result = 4;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject Mayo::Application::staticMetaObject = { {
    &QObject::staticMetaObject,
    qt_meta_stringdata_Mayo__Application.data,
    qt_meta_data_Mayo__Application,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *Mayo::Application::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Mayo::Application::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_Mayo__Application.stringdata0))
        return static_cast<void*>(this);
    if (!strcmp(_clname, "TDocStd_Application"))
        return static_cast< TDocStd_Application*>(this);
    return QObject::qt_metacast(_clname);
}

int Mayo::Application::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 5)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 5;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 5)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 5;
    }
    return _id;
}

// SIGNAL 0
void Mayo::Application::documentAdded(const Mayo::DocumentPtr & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void Mayo::Application::documentAboutToClose(const Mayo::DocumentPtr & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void Mayo::Application::documentNameChanged(const Mayo::DocumentPtr & _t1, const QString & _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void Mayo::Application::documentEntityAdded(const Mayo::DocumentPtr & _t1, Mayo::TreeNodeId _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void Mayo::Application::documentEntityAboutToBeDestroyed(const Mayo::DocumentPtr & _t1, Mayo::TreeNodeId _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
