#ifndef RBX_BUILTIN_TUPLE_HPP
#define RBX_BUILTIN_TUPLE_HPP

#include "memory.hpp"

#include "class/object.hpp"

namespace rubinius {
  class Tuple : public Object {
  public:
    const static object_type type = TupleType;
    const static uintptr_t fields_offset = sizeof(Object) + sizeof(native_int);

    attr_field(full_size, native_int);

    /* Body access */
    Object* field[0];

  public:
    native_int num_fields() const {
      return (full_size() - fields_offset) / sizeof(Object*);
    }

    static void bootstrap(STATE);

    /* Not normally used when allocating a new Tuple instance to avoid double
     * initialization but available for use (eg CompactLookupTable).
     */
    static void initialize(STATE, Tuple* obj) {
      for(native_int i = 0; i < obj->num_fields(); i++) {
        obj->field[i] = cNil;
      }
    }

    static Tuple* create(STATE, native_int fields);
    static Tuple* from(STATE, native_int fields, ...);

    /** Shift all elements leftward, clear old slots. */
    Tuple* lshift_inplace(STATE, Fixnum* shift);


  /** Primitives */
  public:
    // Rubinius.primitive :tuple_allocate
    static Tuple* allocate(STATE, Object* self, Fixnum* fields);

    // Rubinius.primitive :tuple_at
    Object* at_prim(STATE, Fixnum* pos);

    void put_nil(native_int idx) {
      field[idx] = cNil;
    }

    Object* put(STATE, native_int idx, Object* val) {
      field[idx] = val;

      write_barrier(state, val);
      return val;
    }

    // Rubinius.primitive :tuple_put
    Object* put_prim(STATE, Fixnum* idx, Object* val);

    // Rubinius.primitive+ :tuple_fields
    Object* fields_prim(STATE);

    // Rubinius.primitive :tuple_pattern
    static Tuple* pattern(STATE, Fixnum* size, Object* val);

    // Rubinius.primitive :tuple_copy_from
    Tuple* copy_from(STATE, Tuple* other, Fixnum *start, Fixnum *length, Fixnum *dest);

    native_int delete_inplace(native_int start, native_int length, Object* obj);
    // Rubinius.primitive :tuple_delete_inplace
    Fixnum* delete_inplace_prim(STATE, Fixnum *start, Fixnum *length, Object *obj);

    // Rubinius.primitive :tuple_reverse
    Object* reverse(STATE, Fixnum* start, Fixnum* total);

    Tuple* bounds_exceeded_error(STATE, const char* method, int index);

    // Rubinius.primitive :tuple_dup
    Tuple* tuple_dup(STATE);

  public: // Inline Functions
    Object* at(native_int index) {
      if(index < 0 || num_fields() <= index) return cNil;

      return field[index];
    }

    Object* at(STATE, native_int index) {
      return at(index);
    }

  public: // Rubinius Type stuff
    class Info : public TypeInfo {
    public:
      Info(object_type type)
        : TypeInfo(type)
      {
        allow_user_allocate = false;
      }

      virtual void mark(STATE, Object* obj, std::function<void (STATE, Object**)> f);
      virtual void auto_mark(STATE, Object* obj, std::function<void (STATE, Object**)> f) {}
      virtual void show(STATE, Object* self, int level);
      virtual void show_simple(STATE, Object* self, int level);
      virtual size_t object_size(const ObjectHeader* object);
      virtual void before_visit(STATE, Object* o, std::function<void (STATE, Object**)> f);
    };
  };

  class RTuple : public Tuple {
  public:
    const static object_type type = RTupleType;

    Object* at(native_int index) {
      if(index < 0 || num_fields() <= index) return cNil;

      return MemoryHandle::object(reinterpret_cast<VALUE>(field[index]));
    }

    Object* put(STATE, native_int idx, Object* val) {
      reinterpret_cast<VALUE*>(field)[idx] = MemoryHandle::value(val);

      write_barrier(state, val);
      return val;
    }

    static void initialize(STATE, RTuple* obj);

    static RTuple* create(STATE, native_int fields);
    static RTuple* from(STATE, Tuple* tuple);

    // Rubinius.primitive :rtuple_allocate
    static RTuple* allocate(STATE, Object* self, Fixnum* fields);

    // Rubinius.primitive :rtuple_at
    Object* at_prim(STATE, Fixnum* pos);

    // Rubinius.primitive :rtuple_put
    Object* put_prim(STATE, Fixnum* idx, Object* val);

  public:
    class Info : public Tuple::Info {
    public:

      Info(object_type type)
        : Tuple::Info(type)
      {
        allow_user_allocate = false;
      }

      virtual void mark(STATE, Object* obj, std::function<void (STATE, Object**)> f);
      virtual void before_visit(STATE, Object* o, std::function<void (STATE, Object**)> f);
    };
  };
};

#endif
