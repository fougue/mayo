/****************************************************************************
** Meta object code from reading C++ file 'widget_home_files.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.12.8)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../src/app/widget_home_files.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'widget_home_files.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.12.8. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_Mayo__WidgetHomeFiles_t {
    QByteArrayData data[7];
    char stringdata0[103];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_Mayo__WidgetHomeFiles_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_Mayo__WidgetHomeFiles_t qt_meta_stringdata_Mayo__WidgetHomeFiles = {
    {
QT_MOC_LITERAL(0, 0, 21), // "Mayo::WidgetHomeFiles"
QT_MOC_LITERAL(1, 22, 20), // "newDocumentRequested"
QT_MOC_LITERAL(2, 43, 0), // ""
QT_MOC_LITERAL(3, 44, 22), // "openDocumentsRequested"
QT_MOC_LITERAL(4, 67, 23), // "recentFileOpenRequested"
QT_MOC_LITERAL(5, 91, 8), // "FilePath"
QT_MOC_LITERAL(6, 100, 2) // "fp"

    },
    "Mayo::WidgetHomeFiles\0newDocumentRequested\0"
    "\0openDocumentsRequested\0recentFileOpenRequested\0"
    "FilePath\0fp"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_Mayo__WidgetHomeFiles[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       3,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,   29,    2, 0x06 /* Public */,
       3,    0,   30,    2, 0x06 /* Public */,
       4,    1,   31,    2, 0x06 /* Public */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 5,    6,

       0        // eod
};

void Mayo::WidgetHomeFiles::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<WidgetHomeFiles *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->newDocumentRequested(); break;
        case 1: _t->openDocumentsRequested(); break;
        case 2: _t->recentFileOpenRequested((*reinterpret_cast< const FilePath(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (WidgetHomeFiles::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&WidgetHomeFiles::newDocumentRequested)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (WidgetHomeFiles::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&WidgetHomeFiles::openDocumentsRequested)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (WidgetHomeFiles::*)(const FilePath & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&WidgetHomeFiles::recentFileOpenRequested)) {
                *result = 2;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject Mayo::WidgetHomeFiles::staticMetaObject = { {
    &QWidget::staticMetaObject,
    qt_meta_stringdata_Mayo__WidgetHomeFiles.data,
    qt_meta_data_Mayo__WidgetHomeFiles,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *Mayo::WidgetHomeFiles::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Mayo::WidgetHomeFiles::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_Mayo__WidgetHomeFiles.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int Mayo::WidgetHomeFiles::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 3)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 3;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 3)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 3;
    }
    return _id;
}

// SIGNAL 0
void Mayo::WidgetHomeFiles::newDocumentRequested()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void Mayo::WidgetHomeFiles::openDocumentsRequested()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void Mayo::WidgetHomeFiles::recentFileOpenRequested(const FilePath & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
