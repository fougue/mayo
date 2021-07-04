/****************************************************************************
** Meta object code from reading C++ file 'task_manager.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.12.8)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../src/base/task_manager.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'task_manager.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.12.8. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_Mayo__TaskManager_t {
    QByteArrayData data[11];
    char stringdata0[111];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_Mayo__TaskManager_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_Mayo__TaskManager_t qt_meta_stringdata_Mayo__TaskManager = {
    {
QT_MOC_LITERAL(0, 0, 17), // "Mayo::TaskManager"
QT_MOC_LITERAL(1, 18, 7), // "started"
QT_MOC_LITERAL(2, 26, 0), // ""
QT_MOC_LITERAL(3, 27, 12), // "Mayo::TaskId"
QT_MOC_LITERAL(4, 40, 2), // "id"
QT_MOC_LITERAL(5, 43, 12), // "progressStep"
QT_MOC_LITERAL(6, 56, 9), // "stepTitle"
QT_MOC_LITERAL(7, 66, 15), // "progressChanged"
QT_MOC_LITERAL(8, 82, 7), // "percent"
QT_MOC_LITERAL(9, 90, 14), // "abortRequested"
QT_MOC_LITERAL(10, 105, 5) // "ended"

    },
    "Mayo::TaskManager\0started\0\0Mayo::TaskId\0"
    "id\0progressStep\0stepTitle\0progressChanged\0"
    "percent\0abortRequested\0ended"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_Mayo__TaskManager[] = {

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
       5,    2,   42,    2, 0x06 /* Public */,
       7,    2,   47,    2, 0x06 /* Public */,
       9,    1,   52,    2, 0x06 /* Public */,
      10,    1,   55,    2, 0x06 /* Public */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3,    4,
    QMetaType::Void, 0x80000000 | 3, QMetaType::QString,    4,    6,
    QMetaType::Void, 0x80000000 | 3, QMetaType::Int,    4,    8,
    QMetaType::Void, 0x80000000 | 3,    4,
    QMetaType::Void, 0x80000000 | 3,    4,

       0        // eod
};

void Mayo::TaskManager::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<TaskManager *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->started((*reinterpret_cast< Mayo::TaskId(*)>(_a[1]))); break;
        case 1: _t->progressStep((*reinterpret_cast< Mayo::TaskId(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2]))); break;
        case 2: _t->progressChanged((*reinterpret_cast< Mayo::TaskId(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 3: _t->abortRequested((*reinterpret_cast< Mayo::TaskId(*)>(_a[1]))); break;
        case 4: _t->ended((*reinterpret_cast< Mayo::TaskId(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (TaskManager::*)(Mayo::TaskId );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&TaskManager::started)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (TaskManager::*)(Mayo::TaskId , const QString & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&TaskManager::progressStep)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (TaskManager::*)(Mayo::TaskId , int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&TaskManager::progressChanged)) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (TaskManager::*)(Mayo::TaskId );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&TaskManager::abortRequested)) {
                *result = 3;
                return;
            }
        }
        {
            using _t = void (TaskManager::*)(Mayo::TaskId );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&TaskManager::ended)) {
                *result = 4;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject Mayo::TaskManager::staticMetaObject = { {
    &QObject::staticMetaObject,
    qt_meta_stringdata_Mayo__TaskManager.data,
    qt_meta_data_Mayo__TaskManager,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *Mayo::TaskManager::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Mayo::TaskManager::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_Mayo__TaskManager.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int Mayo::TaskManager::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
void Mayo::TaskManager::started(Mayo::TaskId _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void Mayo::TaskManager::progressStep(Mayo::TaskId _t1, const QString & _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void Mayo::TaskManager::progressChanged(Mayo::TaskId _t1, int _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void Mayo::TaskManager::abortRequested(Mayo::TaskId _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void Mayo::TaskManager::ended(Mayo::TaskId _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
