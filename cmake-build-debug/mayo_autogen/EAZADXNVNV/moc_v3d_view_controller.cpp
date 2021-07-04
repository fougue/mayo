/****************************************************************************
** Meta object code from reading C++ file 'v3d_view_controller.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.12.8)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../src/graphics/v3d_view_controller.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'v3d_view_controller.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.12.8. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_Mayo__V3dViewController_t {
    QByteArrayData data[12];
    char stringdata0[159];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_Mayo__V3dViewController_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_Mayo__V3dViewController_t qt_meta_stringdata_Mayo__V3dViewController = {
    {
QT_MOC_LITERAL(0, 0, 23), // "Mayo::V3dViewController"
QT_MOC_LITERAL(1, 24, 20), // "dynamicActionStarted"
QT_MOC_LITERAL(2, 45, 0), // ""
QT_MOC_LITERAL(3, 46, 13), // "DynamicAction"
QT_MOC_LITERAL(4, 60, 9), // "dynAction"
QT_MOC_LITERAL(5, 70, 18), // "dynamicActionEnded"
QT_MOC_LITERAL(6, 89, 10), // "viewScaled"
QT_MOC_LITERAL(7, 100, 10), // "mouseMoved"
QT_MOC_LITERAL(8, 111, 14), // "posMouseInView"
QT_MOC_LITERAL(9, 126, 12), // "mouseClicked"
QT_MOC_LITERAL(10, 139, 15), // "Qt::MouseButton"
QT_MOC_LITERAL(11, 155, 3) // "btn"

    },
    "Mayo::V3dViewController\0dynamicActionStarted\0"
    "\0DynamicAction\0dynAction\0dynamicActionEnded\0"
    "viewScaled\0mouseMoved\0posMouseInView\0"
    "mouseClicked\0Qt::MouseButton\0btn"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_Mayo__V3dViewController[] = {

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
       6,    0,   45,    2, 0x06 /* Public */,
       7,    1,   46,    2, 0x06 /* Public */,
       9,    1,   49,    2, 0x06 /* Public */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3,    4,
    QMetaType::Void, 0x80000000 | 3,    4,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QPoint,    8,
    QMetaType::Void, 0x80000000 | 10,   11,

       0        // eod
};

void Mayo::V3dViewController::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<V3dViewController *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->dynamicActionStarted((*reinterpret_cast< DynamicAction(*)>(_a[1]))); break;
        case 1: _t->dynamicActionEnded((*reinterpret_cast< DynamicAction(*)>(_a[1]))); break;
        case 2: _t->viewScaled(); break;
        case 3: _t->mouseMoved((*reinterpret_cast< const QPoint(*)>(_a[1]))); break;
        case 4: _t->mouseClicked((*reinterpret_cast< Qt::MouseButton(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (V3dViewController::*)(DynamicAction );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&V3dViewController::dynamicActionStarted)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (V3dViewController::*)(DynamicAction );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&V3dViewController::dynamicActionEnded)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (V3dViewController::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&V3dViewController::viewScaled)) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (V3dViewController::*)(const QPoint & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&V3dViewController::mouseMoved)) {
                *result = 3;
                return;
            }
        }
        {
            using _t = void (V3dViewController::*)(Qt::MouseButton );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&V3dViewController::mouseClicked)) {
                *result = 4;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject Mayo::V3dViewController::staticMetaObject = { {
    &QObject::staticMetaObject,
    qt_meta_stringdata_Mayo__V3dViewController.data,
    qt_meta_data_Mayo__V3dViewController,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *Mayo::V3dViewController::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Mayo::V3dViewController::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_Mayo__V3dViewController.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int Mayo::V3dViewController::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
void Mayo::V3dViewController::dynamicActionStarted(DynamicAction _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void Mayo::V3dViewController::dynamicActionEnded(DynamicAction _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void Mayo::V3dViewController::viewScaled()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void Mayo::V3dViewController::mouseMoved(const QPoint & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void Mayo::V3dViewController::mouseClicked(Qt::MouseButton _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
