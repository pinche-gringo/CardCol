#ifndef SIGCEXT_H
#define SIGCEXT_H

//$Id$

// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.


namespace SigC {

template <class R, class P1, class P2, class P3, class P4, class P5, class P6, class P7>
struct FuncSlot7_ {
   typedef typename Trait<R>::type RType;
   typedef RType (*Callback)(P1, P2, P3, P4, P5, P6);
   static RType proxy(typename Trait<P1>::ref p1, typename Trait<P2>::ref p2,
                      typename Trait<P3>::ref p3, typename Trait<P4>::ref p4,
                      typename Trait<P5>::ref p5, typename Trait<P6>::ref p6,
                      typename Trait<P7>::ref p7, void *s) {   
      return ((Callback)(((FuncSlotNode*)s)->func_))(p1, p2, p3, p4, p5, p6, p7); }
};


/// @ingroup Slots
template <class R, class P1, class P2, class P3, class P4, class P5, class P6, class P7>
class Slot7 : public SlotBase {
 public:
   typedef typename Trait<R>::type RType;
   typedef R (*Callback)(P1, P2, P3, P4, P5, P6);
   typedef RType (*Proxy)(typename Trait<P1>::ref p1, typename Trait<P2>::ref p2,
                          typename Trait<P3>::ref p3, typename Trait<P4>::ref p4,
                          typename Trait<P5>::ref p5, typename Trait<P6>::ref p6,
                          typename Trait<P7>::ref p7, void*);
   RType operator () (typename Trait<P1>::ref p1, typename Trait<P2>::ref p2,
                      typename Trait<P3>::ref p3, typename Trait<P4>::ref p4,
                      typename Trait<P5>::ref p5, typename Trait<P6>::ref p6,
                      typename Trait<P7>::ref p7) {
      if (!node_)
         return RType ();
      if (node_->notified_) {
         clear();
         return RType(); }
      return (((Proxy)(static_cast<SlotNode*>(node_)->proxy_))
              (p1, p2, p3, p4, p5, p6, p7, node_)); }
  
   Slot7& operator= (const Slot7 &s) {
      SlotBase::operator=(s);
      return *this; }
  
   Slot7 () : SlotBase () { } 
   Slot7 (const Slot7& s) : SlotBase (s) { }
   Slot7 (SlotNode* node) : SlotBase () { assign (node); }
   Slot7 (Callback callback) : SlotBase () { 
      typedef FuncSlot7_<R, P1, P2, P3, P4, P5, P6, P7> Proxy_;
      assign (new FuncSlotNode ((FuncPtr)&Proxy_::proxy, (FuncPtr)callback)); }
   ~Slot7 () { }
};



template <class R, class P1, class P2, class P3, class P4, class P5, class P6,
   class P7, class Obj>
struct ObjectSlot7_ {
   typedef typename Trait<R>::type RType;
   static RType proxy(typename Trait<P1>::ref p1, typename Trait<P2>::ref p2,
                      typename Trait<P3>::ref p3, typename Trait<P4>::ref p4,
                      typename Trait<P5>::ref p5, typename Trait<P6>::ref p6,
                      typename Trait<P6>::ref p7, void * s) { 
      typedef RType (Obj::*Method)(P1, P2, P3, P4, P5, P6, P7);
      ObjectSlotNode* os = (ObjectSlotNode*)s;
      return ((Obj*)(os->object_)
              ->*(reinterpret_cast<Method>(os->method_)))(p1, p2, p3, p4, p5, p6, p7); }
};


template <class R, class P1, class P2, class P3, class P4, class P5, class P6, class C1>
struct AdaptorBindSlot6_1_ {
   typedef typename Trait<R>::type RType;
   typedef typename Slot7<R, P1, P2, P3, P4, P5, P6, C1>::Proxy Proxy;
   static RType proxy(typename Trait<P1>::ref p1, typename Trait<P2>::ref p2,
                      typename Trait<P3>::ref p3, typename Trait<P4>::ref p4,
                      typename Trait<P5>::ref p5, typename Trait<P6>::ref p6,
                      void *data) {
      typedef AdaptorBindData1_<C1> Data;
      Data& node=*reinterpret_cast<Data*>(data);
      SlotNode* slot=static_cast<SlotNode*> (node.adaptor.slot_.impl ());
      return ((Proxy)(slot->proxy_))
         (p1, p2, p3, p4, p5, p6, node.c1_, slot);
   }
};


/// @ingroup bind
template <class A1, class R, class P1, class P2, class P3, class P4, class P5,
   class P6, class C1>
SigC::Slot6<R, P1, P2, P3, P4, P5, P6>
   bind (const Slot7 <R, P1, P2, P3, P4, P5, P6, C1>& s, A1 a1)
  { 
    typedef AdaptorBindData1_<C1> Data;
    typedef AdaptorBindSlot6_1_<R, P1, P2, P3, P4, P5, P6, C1> Adaptor;
    return reinterpret_cast<SlotNode*>(
       new Data((FuncPtr)(&Adaptor::proxy), s, (FuncPtr)(&Data::dtor), a1));
  }


template <class R, class P1, class P2, class P3, class P4, class P5, class P6,
          class P7, class O1, class O2>
Slot7<R, P1, P2, P3, P4, P5, P6, P7>
slot (O1& obj, R (O2::*method) (P1, P2, P3, P4, P5, P6, P7)) {
   typedef ObjectSlot7_<R, P1, P2, P3, P4, P5, P6, P7, O2> SType;
   O2& obj_of_method = obj;
   return new ObjectSlotNode ((FuncPtr)(&SType::proxy), &obj, &obj_of_method, method);
}

template <class R, class P1, class P2, class P3, class P4, class P5, class P6,
          class P7, class O1, class O2>
Slot7<R, P1, P2, P3, P4, P5, P6, P7>
slot (O1& obj, R (O2::*method) (P1, P2, P3, P4, P5, P6, P7) const) {
   typedef ObjectSlot7_<R, P1, P2, P3, P4, P5, P6, P7, O2> SType;
   O2& obj_of_method = obj;
   return new ObjectSlotNode ((FuncPtr)(&SType::proxy), &obj, &obj_of_method, method);
}

}


#endif
