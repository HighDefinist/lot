#pragma once

#ifdef _MSC_VER
#  pragma warning( disable : 4996 )
#endif

#include <stdexcept>
#include <memory>
#include <iterator>
// "lot", a simplified, faster std::vector. Unlike std::vector or other STL containers, elements are constructed/destructed on internal memory reservation, instead of element insertion/removal/resizing. This means that an element returned by add() already has an undefined, but valid state (either the result of the default constructor, or whatever was the last content). "lot" has basic support for assignment and copy construction, as well as iterators for auto-loops. Move-assignment and -construction are in principle also supported, but Visual C++ appears to have some problems with that in some cases, so it cannot be fully confirmed that it works. If Acheck=true, the array operator uses boundary checks. Tnextsize controls the function which defines the memory allocation pattern during growth.

// "lots" is an adapter which is specifically designed to make GPU memory transfers more pleasent to use, and make CPU debugging easier, by replacing the GPU-memory adapter with a ranged-checked CPU-memory adapter (assuming the rest of the GPU code is also available as CPU code)

#ifndef Acheck_def
# ifdef _DEBUG
#   define Acheck_def true
# else
#   define Acheck_def false
#  endif
#endif

# if defined(__GNUC__)
#   define inli __attribute__((always_inline))
# else
#   define inli __forceinline
# endif

#define Ctypecopy(name) typedef const name C##name
#define Ctypedef(type,name) typedef type name; Ctypecopy(name)

Ctypedef(unsigned int, ui32);
Ctypedef(unsigned long long, ui64);

#define MZ_max(a,b)            (((a) > (b)) ? (a) : (b))
#define MZ_min(a,b)            (((a) < (b)) ? (a) : (b))


namespace std {
  namespace mz {

    template<typename Tidx> struct lot_nextsize {
      Tidx nextsize(Tidx olds) const { return (Tidx)((double)olds*1.5)+4; }
    };

    template <class Tv,bool Acheck = Acheck_def,class Tidx = ui32,class Tnextsize = lot_nextsize<Tidx>> class lot {
    protected:
      class lotIt: public iterator<random_access_iterator_tag,Tv> { // Iterator
      public:
        using difference_Tv = typename std::iterator<std::random_access_iterator_tag,Tv>::difference_type;

        lotIt(): _ptr(nullptr) {}
        lotIt(Tv* rhs): _ptr(rhs) {}
        lotIt(const lotIt &rhs): _ptr(rhs._ptr) {}
        /* inline lotIt& operator=(Tv* rhs) {_ptr = rhs; return *this;} */
        /* inline lotIt& operator=(const lotIt &rhs) {_ptr = rhs._ptr; return *this;} */
        inline lotIt& operator+=(difference_Tv rhs) { _ptr += rhs; return *this; }
        inline lotIt& operator-=(difference_Tv rhs) { _ptr -= rhs; return *this; }
        inline Tv& operator*() const { return *_ptr; }
        inline Tv* operator->() const { return _ptr; }
        inline Tv& operator[](difference_Tv rhs) const { return _ptr[rhs]; }

        inline lotIt& operator++() { ++_ptr; return *this; }
        inline lotIt& operator--() { --_ptr; return *this; }
        inline lotIt operator++(int) { lotIt tmp(*this); ++_ptr; return tmp; }
        inline lotIt operator--(int) { lotIt tmp(*this); --_ptr; return tmp; }
        /* inline lotIt operator+(const lotIt& rhs) {return lotIt(_ptr+rhs.ptr);} */
        inline difference_Tv operator-(const lotIt& rhs) const { return _ptr-rhs._ptr; }
        inline lotIt operator+(difference_Tv rhs) const { return lotIt(_ptr+rhs); }
        inline lotIt operator-(difference_Tv rhs) const { return lotIt(_ptr-rhs); }
      #ifndef __clang__  // Somehow, clang does not support this
        friend inline lotIt operator+(difference_Tv lhs,const lotIt& rhs) { return lotIt(lhs+rhs._ptr); }
        friend inline lotIt operator-(difference_Tv lhs,const lotIt& rhs) { return lotIt(lhs-rhs._ptr); }
      #endif

        inline bool operator==(const lotIt& rhs) const { return _ptr==rhs._ptr; }
        inline bool operator!=(const lotIt& rhs) const { return _ptr!=rhs._ptr; }
        inline bool operator>(const lotIt& rhs) const { return _ptr>rhs._ptr; }
        inline bool operator<(const lotIt& rhs) const { return _ptr<rhs._ptr; }
        inline bool operator>=(const lotIt& rhs) const { return _ptr>=rhs._ptr; }
        inline bool operator<=(const lotIt& rhs) const { return _ptr<=rhs._ptr; }
      private:
        Tv* _ptr;
      };

      //typedef random_access_iterator_tag iterator_category;
      Tidx N,cap;
      void Grow(Tidx nN) { reserve(MZ_max(nN,Tnextsize().nextsize(N))); }
      inli void CopyFrom(const lot& l) {
        resize(l.size());
        for(Tidx i = 0; i<N; i++) v[i] = l.v[i];
      }
      template<typename U> inli void fill_up(const U& item) { v[N-1] = item; }
    #    ifndef useCUDA
      template<typename U,typename ...Args> inli void fill_up(const U& item,Args ...args) {
        auto nn = sizeof...(Args)+1;
        v[N-nn] = item;
        fill_up(args...);
      }
    #    endif
      Tv* v;
    public:
      typedef Tv lot_type;
      // Constructors etc...
      inli ~lot() { Free(); }                                  // Default destructor
      inli lot():cap(0),N(0) {}                                   // Default constructor
      inli lot(Tidx startN) : cap(0),N(0) { resize(startN); }       // Constructor with initial size
      inli lot(const lot& l) : lot() { CopyFrom(l); }                     // Copy constructor
      inli lot& operator=(const lot& l) { CopyFrom(l); return *this; }   // Copy assignment
      inli lot(lot&& l) : cap(l.cap),N(l.N),v(l.v) { l.N = 0; l.cap = 0; } // Move constructor
      inli lot& operator=(lot&& l) {                              // Move assignment
        ::swap(v,l.v);
        ::swap(cap,l.cap);
        N = l.N;
        l.N = 0;
        return *this;
      }
      inli lot(initializer_list<Tv> l):cap(0),N(0) {
        resize((Tidx)l.size());
        uninitialized_copy(l.begin(),l.end(),v); // Use placement new for initialization
      }

      // Element access
      inli Tv* data() { return v; }
      inli Tv& operator[] (Tidx i) const {
        if (Acheck) {
          if (i >= N) {
            throw out_of_range("Lot access out of range!\n");
          }
          else return v[i];
        }
        else return v[i];
      }
      inli Tv& front() { return v[0]; }
      inli Tv& back() { return v[N - 1];
      }
      inli Tv& at(Tidx i) const {
        if (i >= N) throw out_of_range("Lot access out of range!\n"); else return v[i];
      }
      inli Tv& UncheckedAt(Tidx i) const {
        return v[i];
      }

      // Iterators
      inli lotIt begin() const { return lotIt(v); }
      inli lotIt end()   const { return lotIt(v) + N; }

      // Capacity
      inli Tidx size() const { return N; }
      inli Tidx capacity() const { return cap; }
      void reserve(Tidx ncap, bool allowshrink = false) {
        ncap = MZ_max(ncap, allowshrink ? N : cap);
        if (ncap == cap) return;
        Tv* w;
        if (ncap != 0) {
          w = (Tv*)malloc(sizeof(Tv)*ncap);
          memcpy(&w[0], &v[0], sizeof(Tv)*MZ_min(cap,ncap));// Copy content of elements, which should be in both memory areas
          for (Tidx i = cap; i < ncap; i++) new(&w[i]) Tv; // Placement new, to manually call the constructor
        }
        else w = nullptr;
        for (Tidx i = ncap; i < cap; i++) v[i].~Tv(); // Manual call of destructor
        if (cap != 0) free(v);
        cap = ncap;
        v = w;
      }
      void shrink_to_fit() {
        reserve(N, true);
      }

      // Modifiers
      inli void clear() { N = 0; }
      inli void push_back(const Tv& arg) {
        Add(arg);
      }
      inli void pop_back() {
        resize(N - 1);
      }
      inli void resize(Tidx nN) { if (nN > cap)Grow(nN); N = nN; }
      void swap(lot& other) {
        swap(v, other.v);
      }

      // More modifiers
      void Take(lot& l) { Add(l); l.clear(); }
      void Add(const lot& l) {
        auto oldN = N;
        resize(N + l.N);
        for (Tidx i = 0; i < l.N; i++) v[oldN + i] = l[i];
      }
      inli void Add(const Tv& arg) {
        resize(N + 1);
        fill_up(arg);
      }
      Tv* AddEmpty() {
        resize(N + 1);
        return &v[N - 1];
      }
      template<typename ...Args> inli void Add(Args ...args) {
        auto nn = (Tidx)sizeof...(Args);
        resize(N + nn);
        fill_up(args...);
      }
      void Free() {
        clear();
        reserve(0, true);
      }
    };


    class pu_bad_alloc: public exception {
    public:
      pu_bad_alloc(char const* const _Message) throw(): exception(_Message) {}
    };

    class pu_runtime_error: public exception {
    public:
      pu_runtime_error(char const* const _Message) throw(): exception(_Message) {}
    };


    template<class Tv,class Tidx = ui32> class adapter_malloc {
      Tv* data = nullptr;
    public:
      void DevDestroy() {
        free(data);
        data = nullptr;
      }
      void DevCreate(Tidx cap) {
        data = (Tv*)malloc(sizeof(Tv)*cap);
      }
      void CopyDevFromHost(Tv* v,Tidx start,Tidx N) {
        memcpy((Tv*)data+start,&v[start],sizeof(Tv)*N);
      }
      void CopyHostFromDev(Tv* v,Tidx start,Tidx N) {
        memcpy(&v[start],(Tv*)data+start,sizeof(Tv)*N);
      }
      bool isInit() {
        return data!=nullptr;
      }
      Tv*& gV() {
        return data;
      }
    };

    template<class Tv,bool Acheck = Acheck_def,class Tidx = ui32> class adapter_lot {
      lot<Tv,Acheck> data;
    public:
      void DevDestroy() {
        data.Free();
      }
      void DevCreate(Tidx cap) {
        data.reserve(cap);
        data.resize(cap);
      }
      void CopyDevFromHost(Tv* v,Tidx start,Tidx N) {
        Tv* memory = data.data();
        memcpy(memory+start,&v[start],sizeof(Tv)*N);
      }
      void CopyHostFromDev(Tv* v,Tidx start,Tidx N) {
        Tv* memory = data.data();
        memcpy(&v[start],memory+start,sizeof(Tv)*N);
      }
      bool isInit() {
        return data.capacity()!=0;
      }
      lot<Tv>& gV() {
        return data;
      }
    };

  #  ifdef __CUDACC__
    template<class Tv,bool Acheck = Acheck_def,class Tidx = ui32> class adapter_cuda {
      Tv* data = nullptr;
    public:
      void DevDestroy() {
        cudaFree(data);
        data = nullptr;
      }
      void DevCreate(Tidx cap) {
        cudaMalloc(&data,cap*sizeof(Tv));
      }
      void CopyDevFromHost(Tv* v,Tidx start,Tidx N) {
        cudaMemcpy((Tv*)data+start,&v[start],sizeof(Tv)*N,cudaMemcpyHostToDevice);
      }
      void CopyHostFromDev(Tv* v,Tidx start,Tidx N) {
        // For debugging: Show memory alignment
        //i64 A = (i64)&(v[start]);
        //i64 B = (i64)((Tv*)data+start);
        //yprintf("[%-%]",A%16,B%16);
        cudaMemcpy(&v[start],(Tv*)data+start,sizeof(Tv)*N,cudaMemcpyDeviceToHost);
      }
      bool isInit() {
        return data!=nullptr;
      }
      Tv*& gV() {
        return data;
      }
    };
  #  endif

    template <class DeviceAdapter,class Tv,bool Acheck = Acheck_def,class Tidx = ui32,class Tnextsize = lot_nextsize<Tidx>> class lots: public lot<Tv,Acheck,Tidx,Tnextsize> {
      DeviceAdapter Adapter;
      Tidx devCap = 0;
      void DevReserve(Tidx newDevCap) {
        if(newDevCap!=devCap) {
          if(Adapter.isInit())Adapter.DevDestroy();
          if(newDevCap!=0)Adapter.DevCreate(newDevCap);
          if(Adapter.isInit()) devCap = newDevCap; else {
            devCap = 0;
            throw pu_bad_alloc("Memory reservation on device failed");
          }
        }
      }
    public:
      inli Tidx gDevCapacity() {
        return devCap;
      }
      void DevFromHost(Tidx start,Tidx N_) {
        DevInit();
        if(N_>this->cap) throw pu_runtime_error("lots::DevFromHost: Copy size exceeds allocated memory");
        if(devCap!=this->cap) throw pu_runtime_error("lots::DevFromHost: Device was not initialized");
        Adapter.CopyDevFromHost(this->v,start,N_);
      }
      void HostFromDev(Tidx start,Tidx N_) {
        if(start>=this->cap) return;
        if(N_<=0) return;
        N_ = MZ_min(N_,this->cap-start);
        if(N_>this->cap) throw pu_runtime_error("lots::HostFromDev: Copy size exceeds allocated memory");
        if(devCap!=this->cap) throw pu_runtime_error("lots::HostFromDev: Device was not initialized");
        else Adapter.CopyHostFromDev(this->v,start,N_);
      }
      void DevFromHost() {
        DevFromHost(0,this->N);
      }
      void HostFromDev() {
        HostFromDev(0,this->N);
      }
      void DevFromHost(Tidx nCopy) {
        DevFromHost(0,nCopy);
      }
      void HostFromDev(Tidx nCopy) {
        HostFromDev(0,nCopy);
      }
      decltype(Adapter.gV())& DevGet() {
        return Adapter.gV();
      }
      void DevInit() {
        DevReserve(this->cap);
      }
      void ReserveSize(Tidx nSize) {
        this->reserve(nSize,true);
        this->resize(nSize);
        DevInit();
      }
      void DevFree() {
        if(Adapter.isInit()) Adapter.DevDestroy();
      }
      bool DevIsInit() {
        return Adapter.isInit();
      }
      ~lots() { DevFree(); }
      using lot<Tv,Acheck,Tidx,Tnextsize>::lot;  // Inherits constructors of lot

      //inli lots(): lot<Tv,Acheck,Tidx,Tnextsize>() {}                                   // Default constructor
      //inli lots(Tidx startN) : lot(startN) {}
      //inli lot(const lot& l) : lot() { CopyFrom(l); }                     // Copy constructor
      //inli lot& operator=(const lot& l) { CopyFrom(l); return *this; }   // Copy assignment
      //inli lot(lot&& l) : cap(l.cap),N(l.N),v(l.v) { l.N = 0; l.cap = 0; } // Move constructor
      //inli lot& operator=(lot&& l) {                              // Move assignment
      //  swap(v,l.v);
      //  swap(cap,l.cap);
      //  N = l.N;
      //  l.N = 0;
      //  return *this;
      //}
      //inli lot(initializer_list<Tv> l):cap(0),N(0) {
      //  resize((Tidx)l.size());
      //  uninitialized_copy(l.begin(),l.end(),v); // Use placement new for initialization
      //}

    };

  }
}







// Utility macros which access the interal variable directly. They also implicitly perform basic boundary checks, so using them should be ok. x=reference to array slot, aa=Name of the lot to iterate over, ii=iteration integer, v=array name for direct access (if missing, identical to aa)
#define for_lot_xv(x,aa,ii,v) {const ui32 __sz_=aa.size(); auto __zp_=aa.begin().v; const auto v=__zp_; { for(ui32 ii=0; ii<__sz_; ii++){ auto &x=aa[ii];

#define for_lot_x(x,aa,ii) {const ui32 __sz_=aa.size(); auto __zp_=aa.begin().v; const auto aa=__zp_; { for(ui32 ii=0; ii<__sz_; ii++){ auto &x=aa[ii];

#define for_lot(aa,ii) {const ui32 __sz_=aa.size(); auto __zp_=aa.begin().v; const auto aa=__zp_; { for(ui32 ii=0; ii<__sz_; ii++){ 

#define for_lot_ab(aa,ii,i1,i2) {\
  const ui32 __sz_=aa.size(); \
  auto __zp_=aa.begin().v; \
  const auto aa=__zp_; \
  const ui32 __i1_=MZ_max(0,i1); \
  const ui32 __i2_=MZ_min(__sz_,i2);\
  { \
    for(ui32 ii=__i1_; ii<__i2_; ii++){ 
#define for_end }}}

