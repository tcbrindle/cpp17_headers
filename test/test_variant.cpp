
#define STX_NO_STD_VARIANT
#include <stx/variant.hpp>
#include <assert.h>
#include <string>
#include <vector>
#include <memory>
#include <iostream>
#include <array>
#include <tuple>
#include <mutex>

namespace se=stx;

void initial_is_first_type(){
    std::cout<<__FUNCTION__<<std::endl;
    se::variant<int> v;
    assert(!v.valueless_by_exception());
    assert(v.index()==0);
    assert(se::get<int>(v)==0);
}

void can_construct_first_type(){
    std::cout<<__FUNCTION__<<std::endl;
    se::variant<int> v(42);
    assert(v.index()==0);
}

void can_get_value_of_first_type(){
    std::cout<<__FUNCTION__<<std::endl;
    se::variant<int> v(42);
    int& i=se::get<int>(v);
    assert(i==42);
}

void can_construct_second_type(){
    std::cout<<__FUNCTION__<<std::endl;
    se::variant<int,std::string> v(std::string("hello"));
    assert(v.index()==1);
    std::string& s=se::get<std::string>(v);
    assert(s=="hello");
}

void can_move_variant(){
    std::cout<<__FUNCTION__<<std::endl;
    se::variant<int,std::string> v(std::string("hello"));
    se::variant<int,std::string> v2(std::move(v));
    assert(v2.index()==1);
    assert(v.index()==-1);
    std::string& s=se::get<std::string>(v2);
    assert(s=="hello");
}

void can_copy_variant(){
    std::cout<<__FUNCTION__<<std::endl;
    se::variant<int,std::string> v(std::string("hello"));
    se::variant<int,std::string> v2(v);
    assert(v2.index()==1);
    assert(v.index()==1);
    std::string& s=se::get<std::string>(v);
    assert(s=="hello");
    std::string& s2=se::get<std::string>(v2);
    assert(s2=="hello");
    assert(&s!=&s2);
}

void can_copy_const_variant(){
    std::cout<<__FUNCTION__<<std::endl;
    se::variant<int,std::string> const v(std::string("hello"));
    se::variant<int,std::string> v2(v);
    assert(v2.index()==1);
    assert(v.index()==1);
    std::string const& s=se::get<std::string>(v);
    assert(s=="hello");
    std::string& s2=se::get<std::string>(v2);
    assert(s2=="hello");
    assert(&s!=&s2);
}

void construction_from_lvalue(){
    std::cout<<__FUNCTION__<<std::endl;
    std::vector<int> vec(42);
    se::variant<std::vector<int>> v(vec);
    assert(vec.size()==42);
    assert(v.index()==0);
    std::vector<int>& vec2=se::get<std::vector<int>>(v);
    assert(&vec2!=&vec);
    assert(vec2.size()==42);
}

void construction_from_const_lvalue(){
    std::cout<<__FUNCTION__<<std::endl;
    std::vector<int> const vec(42);
    se::variant<std::vector<int>> v(vec);
    assert(vec.size()==42);
    assert(v.index()==0);
    std::vector<int>& vec2=se::get<std::vector<int>>(v);
    assert(&vec2!=&vec);
    assert(vec2.size()==42);
}

void move_construction_with_move_only_types(){
    std::cout<<__FUNCTION__<<std::endl;
    std::unique_ptr<int> ui(new int(42));
    se::variant<std::unique_ptr<int>> v(std::move(ui));
    assert(v.index()==0);
    std::unique_ptr<int>& p2=se::get<std::unique_ptr<int>>(v);
    assert(p2);
    assert(*p2==42);
    
    se::variant<std::unique_ptr<int>> v2(std::move(v));
    assert(v.index()==-1);
    assert(v2.index()==0);
    std::unique_ptr<int>& p3=se::get<std::unique_ptr<int>>(v2);
    assert(p3);
    assert(*p3==42);
}

struct CopyCounter{
    unsigned move_construct=0;
    unsigned copy_construct=0;
    unsigned move_assign=0;
    unsigned copy_assign=0;

    CopyCounter() noexcept{}
    CopyCounter(const CopyCounter& rhs) noexcept:
        move_construct(rhs.move_construct),
        copy_construct(rhs.copy_construct+1),
        move_assign(rhs.move_assign),
        copy_assign(rhs.copy_assign)
    {}
    CopyCounter(CopyCounter&& rhs) noexcept:
        move_construct(rhs.move_construct+1),
        copy_construct(rhs.copy_construct),
        move_assign(rhs.move_assign),
        copy_assign(rhs.copy_assign)
    {}
    CopyCounter& operator=(const CopyCounter& rhs) noexcept{
        move_construct=rhs.move_construct;
        copy_construct=rhs.copy_construct;
        move_assign=rhs.move_assign;
        copy_assign=rhs.copy_assign+1;
        return *this;
    }
    CopyCounter& operator=(CopyCounter&& rhs) noexcept{
        move_construct=rhs.move_construct;
        copy_construct=rhs.copy_construct;
        move_assign=rhs.move_assign+1;
        copy_assign=rhs.copy_assign;
        return *this;
    }
};

void copy_assignment_same_type(){
    std::cout<<__FUNCTION__<<std::endl;
    CopyCounter cc;
    se::variant<CopyCounter> v(cc);
    assert(v.index()==0);
    assert(se::get<CopyCounter>(v).copy_construct==1);
    assert(se::get<CopyCounter>(v).move_construct==0);
    assert(se::get<CopyCounter>(v).copy_assign==0);
    assert(se::get<CopyCounter>(v).move_assign==0);

    se::variant<CopyCounter> v2(cc);
    v2=v;
    assert(v2.index()==0);
    assert(se::get<CopyCounter>(v2).copy_construct==1);
    assert(se::get<CopyCounter>(v2).move_construct==0);
    assert(se::get<CopyCounter>(v2).copy_assign==1);
    assert(se::get<CopyCounter>(v2).move_assign==0);
}

struct ThrowingConversion{
    template<typename T>
    operator T() const{
        throw 42;
    }
};

template<typename V>
void empty_variant(V& v){
    try{
        v.template emplace<0>(ThrowingConversion());
    }
    catch(int){}
    assert(v.valueless_by_exception());
}

void copy_assignment_to_empty(){
    std::cout<<__FUNCTION__<<std::endl;
    CopyCounter cc;
    se::variant<CopyCounter> v(cc);
    assert(v.index()==0);
    assert(se::get<CopyCounter>(v).copy_construct==1);
    assert(se::get<CopyCounter>(v).move_construct==0);
    assert(se::get<CopyCounter>(v).copy_assign==0);
    assert(se::get<CopyCounter>(v).move_assign==0);

    se::variant<CopyCounter> v2;
    empty_variant(v2);
    v2=v;
    assert(v.index()==0);
    assert(v2.index()==0);
    assert(se::get<CopyCounter>(v2).copy_construct==2);
    assert(se::get<CopyCounter>(v2).move_construct==0);
    assert(se::get<CopyCounter>(v2).copy_assign==0);
    assert(se::get<CopyCounter>(v2).move_assign==0);
}

struct InstanceCounter{
    static unsigned instances;

    InstanceCounter(){
        ++instances;
    }
    InstanceCounter(InstanceCounter const& rhs){
        ++instances;
    }
    ~InstanceCounter(){
        --instances;
    }
};

unsigned InstanceCounter::instances=0;
    
void copy_assignment_of_diff_types_destroys_old(){
    std::cout<<__FUNCTION__<<std::endl;
    se::variant<InstanceCounter,int> v;
    assert(InstanceCounter::instances==1);
    v=se::variant<InstanceCounter,int>(InstanceCounter());
    assert(v.index()==0);
    assert(InstanceCounter::instances==1);
    se::variant<InstanceCounter,int> v2(42);
    v=v2;
    assert(v.index()==1);
    assert(v2.index()==1);
    assert(se::get<int>(v2)==42);
    assert(se::get<int>(v)==42);
    assert(InstanceCounter::instances==0);
    v=se::variant<InstanceCounter,int>(InstanceCounter());
    assert(InstanceCounter::instances==1);
    se::variant<InstanceCounter,int> const v3(42);
    v=v3;
    assert(v.index()==1);
    assert(v3.index()==1);
    assert(se::get<int>(v3)==42);
    assert(se::get<int>(v)==42);
    assert(InstanceCounter::instances==0);
    std::cout<<__FUNCTION__<<" done"<<std::endl;
}

void copy_assignment_from_empty(){
    std::cout<<__FUNCTION__<<std::endl;
    se::variant<InstanceCounter,int> v=InstanceCounter();
    assert(v.index()==0);
    assert(InstanceCounter::instances==1);
    se::variant<InstanceCounter,int> v2;
    empty_variant(v2);
    v=v2;
    assert(v.index()==-1);
    assert(v2.index()==-1);
    assert(InstanceCounter::instances==0);
}

struct CopyError{};

struct ThrowingCopy{
    int data;
    
    ThrowingCopy():data(0){}
    ThrowingCopy(ThrowingCopy const&){
        throw CopyError();
    }
    ThrowingCopy(ThrowingCopy&&){
        throw CopyError();
    }
    ThrowingCopy operator=(ThrowingCopy const&){
        throw CopyError();
    }
};

void throwing_copy_assign_leaves_target_unchanged(){
    std::cout<<__FUNCTION__<<std::endl;
    se::variant<std::string,ThrowingCopy> v=std::string("hello");
    assert(v.index()==0);
    se::variant<std::string,ThrowingCopy> v2{se::in_place<ThrowingCopy>};
    try{
        v=v2;
        assert(!"Exception should be thrown");
    }
    catch(CopyError&){
    }
    assert(v.index()==0);
    assert(se::get<0>(v)=="hello");
}

void move_assignment_to_empty(){
    std::cout<<__FUNCTION__<<std::endl;
    CopyCounter cc;
    se::variant<CopyCounter> v(cc);
    assert(v.index()==0);
    assert(se::get<CopyCounter>(v).copy_construct==1);
    assert(se::get<CopyCounter>(v).move_construct==0);
    assert(se::get<CopyCounter>(v).copy_assign==0);
    assert(se::get<CopyCounter>(v).move_assign==0);

    se::variant<CopyCounter> v2;
    empty_variant(v2);
    v2=std::move(v);
    assert(v.index()==-1);
    assert(v2.index()==0);
    assert(se::get<CopyCounter>(v2).copy_construct==1);
    assert(se::get<CopyCounter>(v2).move_construct==1);
    assert(se::get<CopyCounter>(v2).copy_assign==0);
    assert(se::get<CopyCounter>(v2).move_assign==0);
}

void move_assignment_same_type(){
    std::cout<<__FUNCTION__<<std::endl;
    CopyCounter cc;
    se::variant<CopyCounter> v(cc);
    assert(v.index()==0);
    assert(se::get<CopyCounter>(v).copy_construct==1);
    assert(se::get<CopyCounter>(v).move_construct==0);
    assert(se::get<CopyCounter>(v).copy_assign==0);
    assert(se::get<CopyCounter>(v).move_assign==0);

    se::variant<CopyCounter> v2(std::move(cc));
    v2=std::move(v);
    assert(v.index()==-1);
    assert(v2.index()==0);
    assert(se::get<CopyCounter>(v2).copy_construct==1);
    assert(se::get<CopyCounter>(v2).move_construct==0);
    assert(se::get<CopyCounter>(v2).copy_assign==0);
    assert(se::get<CopyCounter>(v2).move_assign==1);
}

void move_assignment_of_diff_types_destroys_old(){
    std::cout<<__FUNCTION__<<std::endl;
    se::variant<InstanceCounter,CopyCounter> v;
    assert(InstanceCounter::instances==1);
    empty_variant(v);
    assert(InstanceCounter::instances==0);
    v=se::variant<InstanceCounter,CopyCounter>(InstanceCounter());
    assert(v.index()==0);
    assert(InstanceCounter::instances==1);
    se::variant<InstanceCounter,CopyCounter> v2{CopyCounter()};
    v=std::move(v2);
    assert(v.index()==1);
    assert(v2.index()==-1);
    assert(InstanceCounter::instances==0);
    assert(se::get<CopyCounter>(v).copy_construct==0);
    assert(se::get<CopyCounter>(v).move_construct==2);
    assert(se::get<CopyCounter>(v).copy_assign==0);
    assert(se::get<CopyCounter>(v).move_assign==0);
}

void move_assignment_from_empty(){
    std::cout<<__FUNCTION__<<std::endl;
    se::variant<InstanceCounter,int> v=InstanceCounter();
    assert(v.index()==0);
    assert(InstanceCounter::instances==1);
    se::variant<InstanceCounter,int> v2;
    empty_variant(v2);
    v=std::move(v2);
    assert(v.index()==-1);
    assert(v2.index()==-1);
    assert(InstanceCounter::instances==0);
}

void emplace_construction_by_type(){
    std::cout<<__FUNCTION__<<std::endl;
    const char* const msg="hello";
    se::variant<int,char const*,std::string> v(
        se::in_place<std::string>,msg);
    assert(v.index()==2);
    assert(se::get<2>(v)==msg);
}

void emplace_construction_by_index(){
    std::cout<<__FUNCTION__<<std::endl;
    const char* const msg="hello";
    se::variant<int,char const*,std::string> v(
        se::in_place<2>,msg);
    assert(v.index()==2);
    assert(se::get<2>(v)==msg);
}

void holds_alternative_for_empty_variant(){
    std::cout<<__FUNCTION__<<std::endl;
    se::variant<int,double> v;
    empty_variant(v);
    assert(!se::holds_alternative<int>(v));
    assert(!se::holds_alternative<double>(v));
}

void holds_alternative_for_non_empty_variant(){
    std::cout<<__FUNCTION__<<std::endl;
    se::variant<int,double> v(2.3);
    assert(!se::holds_alternative<int>(v));
    assert(se::holds_alternative<double>(v));
}

void assignment_from_value_to_empty(){
    std::cout<<__FUNCTION__<<std::endl;
    CopyCounter cc;
    se::variant<int,CopyCounter> v;
    v=cc;
    assert(v.index()==1);
    assert(se::get<CopyCounter>(v).copy_construct==1);
    assert(se::get<CopyCounter>(v).move_construct==0);
    assert(se::get<CopyCounter>(v).copy_assign==0);
    assert(se::get<CopyCounter>(v).move_assign==0);
}

void assignment_from_value_to_same_type(){
    std::cout<<__FUNCTION__<<std::endl;
    CopyCounter cc;
    se::variant<int,CopyCounter> v(cc);
    v=cc;
    assert(v.index()==1);
    assert(se::get<CopyCounter>(v).copy_construct==0);
    assert(se::get<CopyCounter>(v).move_construct==0);
    assert(se::get<CopyCounter>(v).copy_assign==1);
    assert(se::get<CopyCounter>(v).move_assign==0);
}

void assignment_from_value_of_diff_types_destroys_old(){
    std::cout<<__FUNCTION__<<std::endl;
    se::variant<InstanceCounter,CopyCounter> v{InstanceCounter()};
    assert(v.index()==0);
    assert(InstanceCounter::instances==1);
    v=CopyCounter();
    assert(v.index()==1);
    assert(InstanceCounter::instances==0);
    assert(se::get<CopyCounter>(v).copy_construct==0);
    assert(se::get<CopyCounter>(v).move_construct==1);
    assert(se::get<CopyCounter>(v).copy_assign==0);
    assert(se::get<CopyCounter>(v).move_assign==0);
}

void emplace_from_value_to_empty(){
    std::cout<<__FUNCTION__<<std::endl;
    const char* const msg="hello";
    se::variant<int,char const*,std::string> v;
    v.emplace<std::string>(msg);
    assert(v.index()==2);
    assert(se::get<2>(v)==msg);
}

void emplace_from_value_to_same_type(){
    std::cout<<__FUNCTION__<<std::endl;
    CopyCounter cc;
    se::variant<int,CopyCounter> v(cc);
    v.emplace<CopyCounter>();
    assert(v.index()==1);
    assert(se::get<CopyCounter>(v).copy_construct==0);
    assert(se::get<CopyCounter>(v).move_construct==0);
    assert(se::get<CopyCounter>(v).copy_assign==0);
    assert(se::get<CopyCounter>(v).move_assign==0);
}

void emplace_from_value_of_diff_types_destroys_old(){
    std::cout<<__FUNCTION__<<std::endl;
    se::variant<InstanceCounter,CopyCounter> v{InstanceCounter()};
    assert(v.index()==0);
    assert(InstanceCounter::instances==1);
    v.emplace<CopyCounter>();
    assert(v.index()==1);
    assert(InstanceCounter::instances==0);
    assert(se::get<CopyCounter>(v).copy_construct==0);
    assert(se::get<CopyCounter>(v).move_construct==0);
    assert(se::get<CopyCounter>(v).copy_assign==0);
    assert(se::get<CopyCounter>(v).move_assign==0);
}

void emplace_by_index_to_empty(){
    std::cout<<__FUNCTION__<<std::endl;
    const char* const msg="hello";
    se::variant<int,char const*,std::string> v;
    v.emplace<2>(msg);
    assert(v.index()==2);
    assert(se::get<2>(v)==msg);
}

void emplace_by_index_to_same_type(){
    std::cout<<__FUNCTION__<<std::endl;
    CopyCounter cc;
    se::variant<int,CopyCounter> v(cc);
    v.emplace<1>();
    assert(v.index()==1);
    assert(se::get<CopyCounter>(v).copy_construct==0);
    assert(se::get<CopyCounter>(v).move_construct==0);
    assert(se::get<CopyCounter>(v).copy_assign==0);
    assert(se::get<CopyCounter>(v).move_assign==0);
}

void emplace_by_index_of_diff_types_destroys_old(){
    std::cout<<__FUNCTION__<<std::endl;
    se::variant<InstanceCounter,CopyCounter> v{InstanceCounter()};
    assert(v.index()==0);
    assert(InstanceCounter::instances==1);
    v.emplace<1>();
    assert(v.index()==1);
    assert(InstanceCounter::instances==0);
    assert(se::get<CopyCounter>(v).copy_construct==0);
    assert(se::get<CopyCounter>(v).move_construct==0);
    assert(se::get<CopyCounter>(v).copy_assign==0);
    assert(se::get<CopyCounter>(v).move_assign==0);
}

void swap_same_type(){
    std::cout<<__FUNCTION__<<std::endl;
    se::variant<int,CopyCounter> v{CopyCounter()};
    assert(se::get<CopyCounter>(v).copy_construct==0);
    assert(se::get<CopyCounter>(v).move_construct==1);
    assert(se::get<CopyCounter>(v).copy_assign==0);
    assert(se::get<CopyCounter>(v).move_assign==0);

    CopyCounter cc;
    se::variant<int,CopyCounter> v2{cc};
    assert(se::get<CopyCounter>(v2).copy_construct==1);
    assert(se::get<CopyCounter>(v2).move_construct==0);
    assert(se::get<CopyCounter>(v2).copy_assign==0);
    assert(se::get<CopyCounter>(v2).move_assign==0);
    v.swap(v2);

    assert(v.index()==1);
    assert(v2.index()==1);
    assert(se::get<CopyCounter>(v).copy_construct==1);
    assert(se::get<CopyCounter>(v).move_construct==0);
    assert(se::get<CopyCounter>(v).copy_assign==0);
    assert(se::get<CopyCounter>(v).move_assign==1);
    assert(se::get<CopyCounter>(v2).copy_construct==0);
    assert(se::get<CopyCounter>(v2).move_construct==2);
    assert(se::get<CopyCounter>(v2).copy_assign==0);
    assert(se::get<CopyCounter>(v2).move_assign==1);
}

void swap_different_types(){
    std::cout<<__FUNCTION__<<std::endl;
    se::variant<int,CopyCounter> v{CopyCounter()};
    assert(se::get<CopyCounter>(v).copy_construct==0);
    assert(se::get<CopyCounter>(v).move_construct==1);
    assert(se::get<CopyCounter>(v).copy_assign==0);
    assert(se::get<CopyCounter>(v).move_assign==0);

    se::variant<int,CopyCounter> v2{42};
    v.swap(v2);

    assert(v.index()==0);
    assert(v2.index()==1);
    assert(se::get<int>(v)==42);
    assert(se::get<CopyCounter>(v2).copy_construct==0);
    std::cout<<"move count="<<se::get<CopyCounter>(v2).move_construct<<std::endl;
    assert(se::get<CopyCounter>(v2).move_construct<=3);
    assert(se::get<CopyCounter>(v2).move_construct>1);
    assert(se::get<CopyCounter>(v2).copy_assign==0);
    assert(se::get<CopyCounter>(v2).move_assign==0);
    v.swap(v2);
    assert(v2.index()==0);
    assert(v.index()==1);
    assert(se::get<int>(v2)==42);
    assert(se::get<CopyCounter>(v).copy_construct==0);
    assert(se::get<CopyCounter>(v).move_construct==4);
    assert(se::get<CopyCounter>(v).copy_assign==0);
    assert(se::get<CopyCounter>(v).move_assign==0);
}

void assign_empty_to_empty(){
    std::cout<<__FUNCTION__<<std::endl;
    se::variant<int> v1,v2;
    empty_variant(v1);
    empty_variant(v2);
    v1=v2;
    assert(v1.index()==-1);
    assert(v2.index()==-1);
}

void swap_empties(){
    std::cout<<__FUNCTION__<<std::endl;
    se::variant<int> v1,v2;
    empty_variant(v1);
    empty_variant(v2);
    v1.swap(v2);
    assert(v1.index()==-1);
    assert(v2.index()==-1);
}

struct VisitorIS{
    int& i;
    std::string& s;

    void operator()(int arg){
        i=arg;
    }
    void operator()(std::string const& arg){
        s=arg;
    }
};

void visit(){
    std::cout<<__FUNCTION__<<std::endl;
    se::variant<int,std::string> v(42);

    int i=0;
    std::string s;
    VisitorIS visitor{i,s};
    se::visit(visitor,v);
    assert(i==42);
    i=0;
    v=std::string("hello");
    se::visit(visitor,v);
    assert(s=="hello");
    try{
        se::variant<int,std::string> v2;
        empty_variant(v2);
        se::visit(visitor,v2);
        assert(!"Visiting empty should throw");
    }
    catch(se::bad_variant_access){}
}

void reference_members(){
    std::cout<<__FUNCTION__<<std::endl;
    int i=42;
    se::variant<int&> v(se::in_place<0>,i);

    assert(v.index()==0);
    assert(&se::get<int&>(v)==&i);
    assert(&se::get<0>(v)==&i);
}

void equality(){
    std::cout<<__FUNCTION__<<std::endl;
    se::variant<int,double,std::string> v(42);
    se::variant<int,double,std::string> v2(4.2);
    se::variant<int,double,std::string> v3(std::string("42"));

    assert(v==v);
    assert(v!=v2);
    assert(v!=v3);
    assert(v2==v2);
    assert(v3==v3);
    se::variant<int,double,std::string> v4(v);
    assert(v==v4);
    v4=std::move(v2);
    assert(v4!=v2);
    assert(v2==v2);
    assert(v!=v2);
    v2=3;
    assert(v!=v2);
}

void less_than(){
    std::cout<<__FUNCTION__<<std::endl;
    se::variant<int,double,std::string> v(42);
    se::variant<int,double,std::string> v2(4.2);
    se::variant<int,double,std::string> v3(std::string("42"));

    assert(!(v<v));
    assert(v>=v);
    assert(v<v2);
    assert(v<v3);
    assert(v2<v3);
    se::variant<int,double,std::string> v4(v);
    assert(!(v4<v));
    assert(!(v<v4));
    v4=std::move(v2);
    assert(v2<v4);
    assert(v2<v);
    assert(v2<v3);
    v2=99;
    assert(v<v2);
    assert(v2<v4);
    assert(v2<v3);
}

void constexpr_variant(){
    std::cout<<__FUNCTION__<<std::endl;
    constexpr se::variant<int> v(42);
    constexpr int i=se::get<int>(v);
    assert(i==42);
    constexpr se::variant<int> v2(se::in_place<0>,42);
    constexpr int i2=se::get<int>(v2);
    assert(i2==42);
    constexpr se::variant<int> v3(se::in_place<int>,42);
    constexpr int i3=se::get<int>(v3);
    assert(i3==42);
    constexpr se::variant<int,double> v4(4.2);
    constexpr int i4=v4.index();
    assert(i4==1);
    constexpr bool b4=v4.valueless_by_exception();
    assert(!b4);
    constexpr se::variant<int,double> v5;
    constexpr int i5=v5.index();
    assert(i5==0);
    constexpr bool b5=v5.valueless_by_exception();
    assert(!b5);
}

struct VisitorISD{
    int& i;
    std::string& s;
    double& d;
    int& i2;
    
    void operator()(int arg,double d_){
        i=arg;
        d=d_;
    }
    void operator()(std::string const& arg,double d_){
        s=arg;
        d=d_;
    }
    void operator()(int arg,int i2_){
        i=arg;
        i2=i2_;
    }
    void operator()(std::string const& arg,int i2_){
        s=arg;
        i2=i2_;
    }
};

template<size_t>
struct MarkerArg{};

struct ThreeVariantVisitor{

    size_t a1,a2,a3;

    ThreeVariantVisitor():
        a1(0),a2(0),a3(0)
    {}

    template<size_t A1,size_t A2,size_t A3>
    void operator()(MarkerArg<A1>,MarkerArg<A2>,MarkerArg<A3>){
        a1=A1;
        a2=A2;
        a3=A3;
    }
};


void multivisitor(){
    std::cout<<__FUNCTION__<<std::endl;
    se::variant<int,char,std::string> v(42);
    se::variant<double,int> v2(4.2);

    int i=0;
    std::string s;
    double d=0;
    int i2=0;
    VisitorISD visitor{i,s,d,i2};
    se::visit(visitor,v,v2);
    assert(i==42);
    assert(s=="");
    assert(d==4.2);
    assert(i2==0);
    i=0;
    d=0;
    v=std::string("hello");
    assert(v.index()==2);
    v2=37;
    se::visit(visitor,v,v2);
    assert(i==0);
    assert(s=="hello");
    assert(d==0);
    assert(i2==37);

    se::variant<double,int> v3;
    empty_variant(v3);
    try{
        se::visit(visitor,v,v3);
        assert(!"Visiting empty should throw");
    }
    catch(se::bad_variant_access){}

    se::variant<MarkerArg<0>,MarkerArg<1> > mv1{MarkerArg<1>()};
    se::variant<MarkerArg<10>,MarkerArg<11>,MarkerArg<21> > mv2{MarkerArg<21>()};
    se::variant<MarkerArg<100>, MarkerArg<101>, MarkerArg<201>, MarkerArg<301>>
        mv3{MarkerArg<100>()};

    ThreeVariantVisitor tvv;

    se::visit(tvv,mv1,mv2,mv3);

    assert(tvv.a1==1);
    assert(tvv.a2==21);
    assert(tvv.a3==100);
}

void sizes(){
    std::cout<<__FUNCTION__<<std::endl;
    std::cout<<"variant<char>:"<<sizeof(se::variant<char>)<<std::endl;
    std::cout<<"variant<char,int>:"<<sizeof(se::variant<char,int>)<<std::endl;
    std::cout<<"int:"<<sizeof(int)<<std::endl;
    std::cout<<"variant<char,double>:"<<sizeof(se::variant<char,double>)<<std::endl;
    std::cout<<"double:"<<sizeof(double)<<std::endl;
    std::cout<<"variant<char,std::string>:"<<sizeof(se::variant<char,std::string>)<<std::endl;
    std::cout<<"std::string:"<<sizeof(std::string)<<std::endl;
    std::cout<<"variant<char,std::pair<int,int>>:"<<sizeof(se::variant<char,std::pair<int,int>>)<<std::endl;
    std::cout<<"std::pair<int,int>:"<<sizeof(std::pair<int,int>)<<std::endl;
    std::cout<<"variant<char,std::pair<char,char>>:"<<sizeof(se::variant<char,std::pair<char,char>>)<<std::endl;
    std::cout<<"std::pair<char,char>:"<<sizeof(std::pair<char,char>)<<std::endl;
std::cout<<"variant<char,std::pair<double,char>>:"<<sizeof(se::variant<char,std::pair<double,char>>)<<std::endl;
    std::cout<<"std::pair<double,char>:"<<sizeof(std::pair<double,char>)<<std::endl;
    std::cout<<"variant<double,std::tuple<int,int,int>>:"<<sizeof(se::variant<double,std::tuple<int,int,int>>)<<std::endl;
    std::cout<<"std::tuple<int,int,int>:"<<sizeof(std::tuple<int,int,int>)<<std::endl;
}

void duplicate_types(){
    std::cout<<__FUNCTION__<<std::endl;
    se::variant<int,int> v(42);
    // assert(se::get<int>(v)==42);
    assert(v.index()==0);
    assert(se::get<0>(v)==42);

    se::variant<int,int> v2(se::in_place<1>,42);
    assert(v2.index()==1);
    assert(se::get<1>(v2)==42);
    // assert(se::get<int>(v2)==42);
}
struct NonMovable{
    int i;
    NonMovable():i(42){}
    NonMovable(NonMovable&&)=delete;
    NonMovable& operator=(NonMovable&&)=delete;
};


void non_movable_types(){
    std::cout<<__FUNCTION__<<std::endl;
    se::variant<NonMovable> v{se::in_place<0>};
    assert(se::get<0>(v).i==42);
    se::get<0>(v).i=37;
    v.emplace<NonMovable>();
    assert(se::get<0>(v).i==42);
}

void direct_init_reference_member(){
    std::cout<<__FUNCTION__<<std::endl;
    int i=42;
    se::variant<int&> v(i);
    assert(&se::get<int&>(v)==&i);
}

void reference_types_preferred_for_lvalue(){
    std::cout<<__FUNCTION__<<std::endl;
    int i=42;
    se::variant<int,int&> v(i);
    assert(v.index()==1);

    se::variant<int,int&> v2(42);
    assert(v2.index()==0);
}

void construction_with_conversion(){
    std::cout<<__FUNCTION__<<std::endl;
    se::variant<int,std::string> v("hello");
    assert(v.index()==1);
    assert(se::get<1>(v)=="hello");
}

void assignment_with_conversion(){
    std::cout<<__FUNCTION__<<std::endl;
    se::variant<int,std::string> v;
    v="hello";
    assert(v.index()==1);
    assert(se::get<1>(v)=="hello");
}

void visitor_with_non_void_return(){
    std::cout<<__FUNCTION__<<std::endl;
    se::variant<int> v(42);
    assert(visit([](auto i){return i*2;},v)==84);
}

void multi_visitor_with_non_void_return(){
    std::cout<<__FUNCTION__<<std::endl;
    se::variant<int> v(42);
    se::variant<double> v2(4.2);

    assert(visit([](auto i,auto j){return i+j;},v,v2)==46.2);
}

typedef se::variant< std::vector< int >, std::vector< double > > vv;
unsigned foo(vv v){
    return v.index();
}

void initialization_with_initializer_list(){
    std::cout<<__FUNCTION__<<std::endl;
    se::variant<std::vector<int>> v{1,2,3,4};
    assert(v.index()==0);
    assert(se::get<0>(v).size()==4);

    assert(foo({ 1, 2, 3 })==0); // OK
    assert(foo({ 1.2, 3.4, 5.6 })==1); // OK

    // se::variant< char, std::string > q { { 999 } }; // error: can’t deduce T.
    se::variant< char, std::string > r = { 999 }; // valid, but overflows.
    se::variant< char, std::string > s = { 'a', 'b', 'c' }; // error.


}

struct vector_type;
typedef se::variant<int,double,std::string,vector_type> JSON;
struct vector_type{
    std::vector<JSON> vec;
    template<typename T>
    vector_type(std::initializer_list<T> list):
        vec(list.begin(),list.end()){}
    vector_type(std::initializer_list<JSON> list):
        vec(list.begin(),list.end()){}
};


void json(){
    std::cout<<__FUNCTION__<<std::endl;
    JSON v1{1};
    JSON v2{4.2};
    JSON v3{"hello"};
    JSON v4{{1,2,3}};
    assert(v4.index()==3);
    JSON v5{vector_type{1,2,"hello"}};
}

void nothrow_assign_to_variant_holding_type_with_throwing_move_ok(){
    std::cout<<__FUNCTION__<<std::endl;
    se::variant<ThrowingCopy,int> v{se::in_place<0>};
    v=42;
    assert(v.index()==1);
    assert(se::get<1>(v)==42);
}

void maybe_throw_assign_to_variant_holding_type_with_throwing_move_ok(){
    std::cout<<__FUNCTION__<<std::endl;
    se::variant<ThrowingCopy,std::string> v{se::in_place<0>};
    v="hello";
    assert(v.index()==1);
    assert(se::get<1>(v)=="hello");
}

void throwing_assign_from_type_leaves_variant_unchanged(){
    std::cout<<__FUNCTION__<<std::endl;
    se::variant<ThrowingCopy,std::string> v{"hello"};
    try{
        v=ThrowingCopy();
        assert(!"Should throw");
    }
    catch(CopyError&){}
    assert(v.index()==1);
    assert(se::get<1>(v)=="hello");
}

void can_emplace_nonmovable_type_when_other_nothrow_movable(){
    std::cout<<__FUNCTION__<<std::endl;
    se::variant<std::string,NonMovable> v{"hello"};
    v.emplace<1>();
    assert(v.index()==1);
}

struct NonMovableThrower{
    NonMovableThrower(int i){
        if(i==42)
            throw CopyError();
    }
    
    NonMovableThrower(NonMovableThrower&&)=delete;
    NonMovableThrower& operator=(NonMovableThrower&&)=delete;
};


void throwing_emplace_from_nonmovable_type_leaves_variant_empty(){
    std::cout<<__FUNCTION__<<std::endl;
    se::variant<NonMovableThrower,std::string> v{"hello"};
    try{
        v.emplace<NonMovableThrower>(42);
        assert(!"Should throw");
    }
    catch(CopyError&){}
    assert(v.index()==-1);
}

void throwing_emplace_when_stored_type_can_throw_leaves_variant_empty(){
    std::cout<<__FUNCTION__<<std::endl;
    se::variant<NonMovableThrower,ThrowingCopy> v{
        se::in_place<ThrowingCopy>};
    se::get<1>(v).data=21;
    try{
        v.emplace<NonMovableThrower>(42);
        assert(!"Should throw");
    }
    catch(CopyError&){}
    assert(v.index()==-1);
}

struct MayThrowA{
    int data;
    MayThrowA(int i):
        data(i){}
    MayThrowA(MayThrowA const& other):
        data(other.data){}
};

struct MayThrowB{
    int data;
    MayThrowB(int i):
        data(i){}
    MayThrowB(MayThrowB const& other):
        data(other.data){}
};

void after_assignment_which_triggers_backup_storage_can_assign_variant(){
    std::cout<<__FUNCTION__<<std::endl;
    se::variant<MayThrowA,MayThrowB> v{MayThrowA(23)};
    v.emplace<MayThrowB>(42);
    assert(v.index()==1);
    assert(se::get<1>(v).data==42);
    se::variant<MayThrowA,MayThrowB> v2=v;
    assert(v2.index()==1);
    assert(se::get<1>(v2).data==42);
    v=MayThrowA(23);
    assert(v.index()==0);
    assert(se::get<0>(v).data==23);
    v2=v;
    assert(v2.index()==0);
    assert(se::get<0>(v2).data==23);
    v2=MayThrowB(19);
    assert(v2.index()==1);
    assert(se::get<1>(v2).data==19);
    v=v2;
    assert(v2.index()==1);
    assert(se::get<1>(v2).data==19);
}

void backup_storage_and_local_backup(){
    std::cout<<__FUNCTION__<<std::endl;
    se::variant<std::string,ThrowingCopy> v{"hello"};
    assert(v.index()==0);
    assert(se::get<0>(v)=="hello");
    try{
        v=ThrowingCopy();
        assert(!"Should throw");
    }
    catch(CopyError&){}
    assert(v.index()==0);
    assert(se::get<0>(v)=="hello");
}

struct LargeNoExceptMovable{
    char buf[512];

    LargeNoExceptMovable() noexcept{}
    LargeNoExceptMovable(LargeNoExceptMovable&&) noexcept{}
    LargeNoExceptMovable(LargeNoExceptMovable const&&) noexcept{}
    LargeNoExceptMovable& operator=(LargeNoExceptMovable&&) noexcept{
        return *this;
    }
    LargeNoExceptMovable& operator=(LargeNoExceptMovable const&&) noexcept{
        return *this;
    }
};


void large_noexcept_movable_and_small_throw_movable(){
    std::cout<<__FUNCTION__<<std::endl;
    se::variant<LargeNoExceptMovable,MayThrowA,MayThrowB> v{
        LargeNoExceptMovable()};
    v=MayThrowB(21);
    v=LargeNoExceptMovable();
    v=MayThrowA(12);
    assert(sizeof(v)<(2*sizeof(LargeNoExceptMovable)));
}

struct LargeMayThrowA{
    char dummy[16];
    LargeMayThrowA();
    LargeMayThrowA(LargeMayThrowA const&){}
};


void construct_small_with_large_throwables(){
    std::cout<<__FUNCTION__<<std::endl;
    se::variant<std::string,LargeMayThrowA,MayThrowB> v{
        MayThrowB(12)};
    v="hello";
    assert(v.index()==0);
    assert(se::get<0>(v)=="hello");
    v=MayThrowB(42);
    std::cout<<"size of v="<<sizeof(v)<<std::endl;
    std::cout<<"size of string="<<sizeof(std::string)<<std::endl;
    std::cout<<"size of LargeMayThrowA="<<sizeof(LargeMayThrowA)<<std::endl;
    std::cout<<"size of MayThrowB="<<sizeof(MayThrowB)<<std::endl;
}

void if_emplace_throws_variant_is_valueless(){
    std::cout<<__FUNCTION__<<std::endl;
    se::variant<int> v;
    assert(!v.valueless_by_exception());
    assert(v.index()==0);
    try{
        v.emplace<0>(ThrowingConversion());
        assert(!"Conversion should throw");
    }
    catch(...){}
    assert(v.index()==-1);
    assert(v.valueless_by_exception());
}

#ifndef __MINGW32__
void properties(){
    std::cout<<__FUNCTION__<<std::endl;
    static_assert(!std::is_default_constructible<se::variant<>>::value);
    static_assert(std::is_copy_constructible<se::variant<>>::value);  // or should this be false?
    static_assert(std::is_copy_constructible<se::variant<int>>::value);
    static_assert(!std::is_copy_constructible<se::variant<std::mutex,int>>::value);
    static_assert(!std::is_move_constructible<se::variant<std::mutex,int>>::value);
    static_assert( std::is_nothrow_move_constructible<se::variant<std::string>>::value);
    static_assert(!std::is_move_assignable<se::variant<std::mutex,int>>::value);
    static_assert(!std::is_copy_assignable<se::variant<std::mutex,int>>::value);
    static_assert(std::is_move_assignable<se::variant<std::string,int>>::value);
    static_assert(std::is_copy_assignable<se::variant<std::string,int>>::value);
    static_assert(std::is_nothrow_move_assignable<se::variant<std::string,int>>::value);
    static_assert(std::is_move_assignable<se::variant<ThrowingCopy,int>>::value);
    static_assert(!std::is_nothrow_move_assignable<se::variant<ThrowingCopy,int>>::value);
    static_assert(!noexcept(se::variant<ThrowingCopy,int>().swap(std::declval<se::variant<ThrowingCopy,int>&>())));
    static_assert(noexcept(se::variant<int,double>().swap(std::declval<se::variant<int,double>&>())));
}
#endif

void variant_of_references(){
    std::cout<<__FUNCTION__<<std::endl;
    static int i=42;
    constexpr se::variant<int&> vi(i);
    static_assert(&se::get<0>(vi)==&i);
    constexpr se::variant<std::string&,int&> vi2(i);
    static_assert(&se::get<1>(vi2)==&i);
    constexpr se::variant<const int&> vi3(i);
    static_assert(&se::get<0>(vi3)==&i);
}

void variant_size(){
    std::cout<<__FUNCTION__<<std::endl;
    static_assert(se::variant_size<se::variant<int>>::value==1);
    static_assert(se::variant_size<se::variant<int,double>>::value==2);
    static_assert(se::variant_size<se::variant<std::string,int,double>>::value==3);
    static_assert(se::variant_size<se::variant<int,double,int&,const std::string>>::value==4);
    static_assert(se::variant_size<const se::variant<int>>::value==1);
    static_assert(se::variant_size<const se::variant<int,double>>::value==2);
    static_assert(se::variant_size<const se::variant<std::string,int,double>>::value==3);
    static_assert(se::variant_size<const se::variant<int,double,int&,const std::string>>::value==4);
    static_assert(se::variant_size<volatile se::variant<int>>::value==1);
    static_assert(se::variant_size<volatile se::variant<int,double>>::value==2);
    static_assert(se::variant_size<volatile se::variant<std::string,int,double>>::value==3);
    static_assert(se::variant_size<volatile se::variant<int,double,int&,const std::string>>::value==4);
    static_assert(se::variant_size<volatile const se::variant<int>>::value==1);
    static_assert(se::variant_size<volatile const se::variant<int,double>>::value==2);
    static_assert(se::variant_size<volatile const se::variant<std::string,int,double>>::value==3);
    static_assert(se::variant_size<volatile const se::variant<int,double,int&,const std::string>>::value==4);
}

void variant_alternative(){
    std::cout<<__FUNCTION__<<std::endl;
    static_assert(std::is_same<se::variant_alternative<0,se::variant<int>>::type,int>::value);
    static_assert(std::is_same<se::variant_alternative<0,se::variant<int,std::string>>::type,int>::value);
    static_assert(std::is_same<se::variant_alternative<1,se::variant<int,std::string>>::type,std::string>::value);
    static_assert(std::is_same<se::variant_alternative<0,se::variant<const int>>::type,const int>::value);
    static_assert(std::is_same<se::variant_alternative<0,se::variant<int&,std::string>>::type,int&>::value);
    static_assert(std::is_same<se::variant_alternative<2,se::variant<int,std::string,const double&>>::type,const double&>::value);
    static_assert(std::is_same<se::variant_alternative_t<0,se::variant<int,std::string,const double&>>,int>::value);
    
    static_assert(std::is_same<se::variant_alternative<0,const se::variant<int>>::type,const int>::value);
    static_assert(std::is_same<se::variant_alternative<0,const se::variant<int,std::string>>::type,const int>::value);
    static_assert(std::is_same<se::variant_alternative<1,const se::variant<int,std::string>>::type,const std::string>::value);
    static_assert(std::is_same<se::variant_alternative<0,const se::variant<const int>>::type,const int>::value);
    static_assert(std::is_same<se::variant_alternative<0,const se::variant<int&,std::string>>::type,int&>::value);
    static_assert(std::is_same<se::variant_alternative<2,const se::variant<int,std::string,const double&>>::type,const double&>::value);
    static_assert(std::is_same<se::variant_alternative_t<0,const se::variant<int,std::string,const double&>>,const int>::value);

    static_assert(std::is_same<se::variant_alternative<0,volatile const se::variant<int>>::type,volatile const int>::value);
    static_assert(std::is_same<se::variant_alternative<0,volatile const se::variant<int,std::string>>::type,volatile const int>::value);
    static_assert(std::is_same<se::variant_alternative<1,volatile const se::variant<int,std::string>>::type,volatile const std::string>::value);
    static_assert(std::is_same<se::variant_alternative<0,volatile const se::variant<const int>>::type,volatile const int>::value);
    static_assert(std::is_same<se::variant_alternative<0,volatile const se::variant<int&,std::string>>::type,int&>::value);
    static_assert(std::is_same<se::variant_alternative<2,volatile const se::variant<int,std::string,const double&>>::type,const double&>::value);
    static_assert(std::is_same<se::variant_alternative_t<0,volatile const se::variant<int,std::string,const double&>>,volatile const int>::value);

    static_assert(std::is_same<se::variant_alternative<0,volatile se::variant<int>>::type,volatile int>::value);
    static_assert(std::is_same<se::variant_alternative<0,volatile se::variant<int,std::string>>::type,volatile int>::value);
    static_assert(std::is_same<se::variant_alternative<1,volatile se::variant<int,std::string>>::type,volatile std::string>::value);
    static_assert(std::is_same<se::variant_alternative<0,volatile se::variant<const int>>::type,volatile const int>::value);
    static_assert(std::is_same<se::variant_alternative<0,volatile se::variant<int&,std::string>>::type,int&>::value);
    static_assert(std::is_same<se::variant_alternative<2,volatile se::variant<int,std::string,const double&>>::type,const double&>::value);
    static_assert(std::is_same<se::variant_alternative_t<0,volatile se::variant<int,std::string,const double&>>,volatile int>::value);
}

void npos(){
    std::cout<<__FUNCTION__<<std::endl;
    static_assert(se::variant_npos==(size_t)-1);
    static_assert(std::is_same<decltype(se::variant_npos),const size_t>::value);
}

void holds_alternative(){
    std::cout<<__FUNCTION__<<std::endl;
    constexpr se::variant<int> vi(42);
    static_assert(se::holds_alternative<int>(vi));
    constexpr se::variant<int,double> vi2(42);
    static_assert(se::holds_alternative<int>(vi2));
    static_assert(!se::holds_alternative<double>(vi2));
    constexpr se::variant<int,double> vi3(4.2);
    static_assert(!se::holds_alternative<int>(vi3));
    static_assert(se::holds_alternative<double>(vi3));

    const se::variant<int,double,std::string> vi4(42);
    assert(se::holds_alternative<int>(vi4));
    assert(!se::holds_alternative<double>(vi4));
    assert(!se::holds_alternative<std::string>(vi4));

    se::variant<int,double,std::string> vi5("hello42");
    assert(!se::holds_alternative<int>(vi5));
    assert(!se::holds_alternative<double>(vi5));
    assert(se::holds_alternative<std::string>(vi5));
}

void get_with_rvalues(){
    std::cout<<__FUNCTION__<<std::endl;

    int i=42;

    static_assert(se::get<0>(se::variant<int>(42))==42);
    static_assert(std::is_same<decltype(se::get<0>(se::variant<int>(42))),int&&>::value);
    static_assert(se::get<int>(se::variant<int>(42))==42);
    static_assert(std::is_same<decltype(se::get<int>(se::variant<int>(42))),int&&>::value);

    static_assert(se::get<1>(se::variant<double,int,char>(42))==42);
    static_assert(std::is_same<decltype(se::get<1>(se::variant<double,int,char>(42))),int&&>::value);
    static_assert(se::get<int>(se::variant<double,int,char>(42))==42);
    static_assert(std::is_same<decltype(se::get<int>(se::variant<double,int,char>(42))),int&&>::value);

    assert(se::get<0>(se::variant<int&>(i))==42);
    static_assert(std::is_same<decltype(se::get<0>(se::variant<int&>(i))),int&>::value);
    assert(&se::get<0>(se::variant<int&>(i))==&i);
    assert(se::get<int&>(se::variant<int&>(i))==42);
    static_assert(std::is_same<decltype(se::get<int&>(se::variant<int&>(i))),int&>::value);
    assert(&se::get<int&>(se::variant<int&>(i))==&i);
    
    assert(se::get<0>(se::variant<int&&>(std::move(i)))==42);
    static_assert(std::is_same<decltype(se::get<0>(se::variant<int&&>(std::move(i)))),int&&>::value);
    int&& ir=se::get<0>(se::variant<int&&>(std::move(i)));
    assert(&ir==&i);
    assert(se::get<int&&>(se::variant<int&&>(std::move(i)))==42);
    static_assert(std::is_same<decltype(se::get<int&&>(se::variant<int&&>(std::move(i)))),int&&>::value);
    int&& ir2=se::get<int&&>(se::variant<int&&>(std::move(i)));
    assert(&ir2==&i);
    
    se::variant<int> vi(42);

    int&& ri=se::get<0>(std::move(vi));
    int&& ri2=se::get<int>(std::move(vi));
    assert(&ri==&ri2);
    assert(&ri==&se::get<0>(vi));

    se::variant<int,std::string> v2("hello");

    std::string&& rs=se::get<1>(std::move(v2));
    std::string&& rs2=se::get<std::string>(std::move(v2));
    assert(&rs==&rs2);
    assert(&rs==&se::get<1>(v2));
    assert(rs=="hello");
}

void get_with_const_rvalues(){
    std::cout<<__FUNCTION__<<std::endl;

    int i=42;
    
    static_assert(se::get<0>((const se::variant<int>)(42))==42);
    static_assert(std::is_same<decltype(se::get<0>((const se::variant<int>)(42))),const int&&>::value);
    static_assert(se::get<int>((const se::variant<int>)(42))==42);
    static_assert(std::is_same<decltype(se::get<int>((const se::variant<int>)(42))),const int&&>::value);

    static_assert(se::get<1>((const se::variant<double,int,char>)(42))==42);
    static_assert(std::is_same<decltype(se::get<1>((const se::variant<double,int,char>)(42))),const int&&>::value);
    static_assert(se::get<int>((const se::variant<double,int,char>)(42))==42);
    static_assert(std::is_same<decltype(se::get<int>((const se::variant<double,int,char>)(42))),const int&&>::value);
    
    assert(se::get<0>((const se::variant<int&>)(i))==42);
    static_assert(std::is_same<decltype(se::get<0>((const se::variant<int&>)(i))),int&>::value);
    assert(&se::get<0>((const se::variant<int&>)(i))==&i);
    assert(se::get<int&>((const se::variant<int&>)(i))==42);
    static_assert(std::is_same<decltype(se::get<int&>((const se::variant<int&>)(i))),int&>::value);
    assert(&se::get<int&>((const se::variant<int&>)(i))==&i);

    assert(se::get<0>((const se::variant<int&&>)(std::move(i)))==42);
    static_assert(std::is_same<decltype(se::get<0>((const se::variant<int&&>)(std::move(i)))),int&&>::value);
    int&& ir=se::get<0>((const se::variant<int&&>)(std::move(i)));
    assert(&ir==&i);
    assert(se::get<int&&>((const se::variant<int&&>)(std::move(i)))==42);
    static_assert(std::is_same<decltype(se::get<int&&>((const se::variant<int&&>)(std::move(i)))),int&&>::value);
    int&& ir2=se::get<int&&>((const se::variant<int&&>)(std::move(i)));
    assert(&ir2==&i);
    
    const se::variant<int> vi(42);

    const int&& ri=se::get<0>(std::move(vi));
    const int&& ri2=se::get<int>(std::move(vi));
    assert(&ri==&ri2);
    assert(&ri==&se::get<0>(vi));

    const se::variant<int,std::string> v2("hello");

    const std::string&& rs=se::get<1>(std::move(v2));
    const std::string&& rs2=se::get<std::string>(std::move(v2));
    assert(&rs==&rs2);
    assert(&rs==&se::get<1>(v2));
    assert(rs=="hello");
}

void get_if(){
    std::cout<<__FUNCTION__<<std::endl;
    constexpr se::variant<int> cvi(42);
    constexpr se::variant<double,int,char> cvidc(42);
    constexpr se::variant<double,int,char> cvidc2(4.2);

    static_assert(se::get_if<0>(cvi)==&se::get<0>(cvi));
    static_assert(se::get_if<int>(cvi)==&se::get<0>(cvi));
    
    static_assert(!se::get_if<0>(cvidc));
    static_assert(se::get_if<1>(cvidc)==&se::get<1>(cvidc));
    static_assert(!se::get_if<2>(cvidc));
    static_assert(!se::get_if<double>(cvidc));
    static_assert(se::get_if<int>(cvidc)==&se::get<1>(cvidc));
    static_assert(!se::get_if<char>(cvidc));

    static_assert(se::get_if<double>(cvidc2)==&se::get<0>(cvidc2));
    static_assert(!se::get_if<int>(cvidc2));
    static_assert(!se::get_if<char>(cvidc2));

    se::variant<int> vi(42);
    se::variant<double,int,char> vidc(42);
    se::variant<double,int,char> vidc2(4.2);

    assert(se::get_if<0>(vi)==&se::get<0>(vi));
    assert(se::get_if<int>(vi)==&se::get<0>(vi));
    
    assert(!se::get_if<0>(vidc));
    assert(se::get_if<1>(vidc)==&se::get<1>(vidc));
    assert(!se::get_if<2>(vidc));
    assert(!se::get_if<double>(vidc));
    assert(se::get_if<int>(vidc)==&se::get<1>(vidc));
    assert(!se::get_if<char>(vidc));

    assert(se::get_if<double>(vidc2)==&se::get<0>(vidc2));
    assert(!se::get_if<int>(vidc2));
    assert(!se::get_if<char>(vidc2));
}

void constexpr_comparisons(){
    std::cout<<__FUNCTION__<<std::endl;
    constexpr se::variant<int,double,char> vi(42);
    constexpr se::variant<int,double,char> vi2(21);
    constexpr se::variant<int,double,char> vd(2.1);
    constexpr se::variant<int,double,char> vd2(4.2);

    static_assert(vi==vi);
    static_assert(vi!=vi2);
    static_assert(vi>vi2);
    static_assert(vi>=vi2);
    static_assert(vi2<vi);
    static_assert(vi2<=vi);
    static_assert(vd!=vd2);
    static_assert(vd==vd);
    static_assert(vd<vd2);
    static_assert(vd<=vd2);
    static_assert(vd2>vd);
    static_assert(vd2>=vd);
}

struct Identity {
    template <typename T> constexpr T operator()(T x) {
        return x;
    }
};

struct Sum {
    template <typename T, typename U>
    constexpr auto operator()(T x, U y) {
        return x + y;
    }
};

void constexpr_visit(){
    std::cout<<__FUNCTION__<<std::endl;

    constexpr se::variant<int,double> vi(42);
    constexpr se::variant<int,double> vi2(21);

    static_assert(se::visit(Identity(),vi)==42);
    static_assert(se::visit(Sum(),vi,vi2)==63);
}

void variant_with_no_types(){
    std::cout<<__FUNCTION__<<std::endl;
    static_assert(sizeof(se::variant<>)>0);
    static_assert(!std::is_default_constructible<se::variant<>>::value);
}

void monostate(){
    std::cout<<__FUNCTION__<<std::endl;
    static_assert(std::is_trivial<se::monostate>::value);
    static_assert(std::is_nothrow_move_constructible<se::monostate>::value);
    static_assert(std::is_nothrow_copy_constructible<se::monostate>::value);
    static_assert(std::is_nothrow_move_assignable<se::monostate>::value);
    static_assert(std::is_nothrow_copy_assignable<se::monostate>::value);
    constexpr se::monostate m1{},m2{};
    static_assert(m1==m2);
    static_assert(!(m1!=m2));
    static_assert(m1>=m2);
    static_assert(m1<=m2);
    static_assert(!(m1<m2));
    static_assert(!(m1>m2));
}

void hash(){
    std::cout<<__FUNCTION__<<std::endl;

    se::variant<int,std::string> vi(42);
    se::variant<int,std::string> vi2(vi);
    
    std::hash<se::variant<int,std::string>> h;
    static_assert(noexcept(h(vi)));
    static_assert(std::is_same<decltype(h(vi)),size_t>::value);

    assert(h(vi)==h(vi2));

    se::monostate m{};
    std::hash<se::monostate> hm;
    static_assert(noexcept(hm(m)));
    static_assert(std::is_same<decltype(hm(m)),size_t>::value);
}

unsigned allocate_count=0;
unsigned deallocate_count=0;

template <typename T> struct CountingAllocator
{
    using value_type = T;
    using is_always_equal = std::true_type;
    using pointer = T *;
    using const_pointer = T const *;
    using reference = T &;
    using const_reference = T const &;


    template <typename U> struct rebind
    {
        using other = CountingAllocator<U>;
    };

    template <typename U> CountingAllocator(CountingAllocator<U> const &) {}

    CountingAllocator() {}

    T *allocate(size_t count)
    {
        ++allocate_count;
        auto *buf = malloc(count * sizeof(T));
        return static_cast<T *>(buf);
    }
    void deallocate(T *p, size_t)
    {
        if (!p)
            return;
        ++deallocate_count;
        free(p);
    }

    void destroy(T *p) { p->~T(); }

    template <class... Args>
    void construct(T* p, Args&&... args){
        ::new (static_cast<void*>(p)) T(std::forward<Args>(args)...);
    }
};

template <typename T, typename U>
constexpr bool operator==(CountingAllocator<T> const &, CountingAllocator<U> const &)
{
   return true;
}

template <typename T, typename U>
constexpr bool operator!=(CountingAllocator<T> const &, CountingAllocator<U> const &)
{
   return false;
}

struct allocatable{

    bool allocator_supplied=false;
    bool was_moved=false;

    allocatable(){}
    template<typename Alloc>
    allocatable(std::allocator_arg_t,Alloc const&):
        allocator_supplied(true){}

    allocatable(allocatable const&)=default;
    allocatable(allocatable&&):was_moved(true){}

    template<typename Alloc>
    allocatable(std::allocator_arg_t,Alloc const&,allocatable const&):
        allocator_supplied(true){}

    template<typename Alloc>
    allocatable(std::allocator_arg_t,Alloc const&,allocatable &&):
        allocator_supplied(true),was_moved(true){}

};

struct allocatable_no_arg{

    bool allocator_supplied=false;
    bool was_moved=false;

    allocatable_no_arg(){}

    allocatable_no_arg(allocatable_no_arg const&)=default;
    allocatable_no_arg(allocatable_no_arg&&):
        was_moved(true){}
    
    
    template<typename Alloc>
    allocatable_no_arg(Alloc const&):
        allocator_supplied(true){}

    template<typename Alloc>
    allocatable_no_arg(Alloc const&,allocatable_no_arg const&):
        allocator_supplied(true){}

    template<typename Alloc>
    allocatable_no_arg(Alloc const&,allocatable_no_arg &&):
        allocator_supplied(true),was_moved(true){}

};

struct not_allocatable{

    bool allocator_supplied=false;
    bool was_moved=false;

    not_allocatable(){}
    not_allocatable(not_allocatable const&)=default;
    not_allocatable(not_allocatable&&):
        was_moved(true){}

    template<typename Alloc>
    not_allocatable(std::allocator_arg_t,Alloc const&):
        allocator_supplied(true){}

    template<typename Alloc>
    not_allocatable(std::allocator_arg_t,Alloc const&,not_allocatable const&):
        allocator_supplied(true){}
    template<typename Alloc>
    not_allocatable(Alloc const&,not_allocatable &&):
        allocator_supplied(true),was_moved(true){}
};

namespace std{
template<typename Alloc>
struct uses_allocator<allocatable,Alloc>:
        true_type{};

template<typename Alloc>
struct uses_allocator<allocatable_no_arg,Alloc>:
        true_type{};
}

void allocator_default_constructor_no_allocator_support(){
    std::cout<<__FUNCTION__<<std::endl;

    struct MyClass{
        int i;
        MyClass():
            i(42){}
    };

    se::variant<MyClass,std::string> vi{std::allocator_arg_t(),CountingAllocator<MyClass>()};

    assert(allocate_count==0);
    assert(vi.index()==0);
    assert(se::get<0>(vi).i==42);

    se::variant<not_allocatable,std::string> v3{std::allocator_arg_t(),CountingAllocator<int>()};

    assert(allocate_count==0);
    assert(v3.index()==0);
    assert(!se::get<0>(v3).allocator_supplied);
}

void allocator_default_constructor_allocator_arg_support(){
    std::cout<<__FUNCTION__<<std::endl;

    se::variant<allocatable,std::string> v2{std::allocator_arg_t(),CountingAllocator<int>()};

    assert(allocate_count==0);
    assert(v2.index()==0);
    assert(se::get<0>(v2).allocator_supplied);
}

void allocator_default_constructor_no_allocator_arg_support(){
    std::cout<<__FUNCTION__<<std::endl;

    se::variant<allocatable_no_arg,std::string> v2{std::allocator_arg_t(),CountingAllocator<int>()};

    assert(allocate_count==0);
    assert(v2.index()==0);
    assert(se::get<0>(v2).allocator_supplied);
}

void variant_uses_allocator(){
    std::cout<<__FUNCTION__<<std::endl;

    static_assert(
        std::uses_allocator<se::variant<int,std::string>,CountingAllocator<double>>::value,
        "Variant should use allocator");
}
    
void allocator_index_constructor_no_allocator_support(){
    std::cout<<__FUNCTION__<<std::endl;

    struct MyClass{
        int i;
        MyClass():
            i(42){}
    };

    se::variant<MyClass,std::string> vi{std::allocator_arg_t(),CountingAllocator<MyClass>(),se::in_place<0>};

    assert(allocate_count==0);
    assert(vi.index()==0);
    assert(se::get<0>(vi).i==42);

    se::variant<std::string,MyClass> vi2{std::allocator_arg_t(),CountingAllocator<MyClass>(),se::in_place<1>};

    assert(allocate_count==0);
    assert(vi2.index()==1);
    assert(se::get<1>(vi2).i==42);


    se::variant<not_allocatable,std::string> v1{std::allocator_arg_t(),CountingAllocator<int>(),se::in_place<0>};

    assert(allocate_count==0);
    assert(v1.index()==0);
    assert(!se::get<0>(v1).allocator_supplied);

    se::variant<std::string,not_allocatable> v2{std::allocator_arg_t(),CountingAllocator<int>(),se::in_place<1>};

    assert(allocate_count==0);
    assert(v2.index()==1);
    assert(!se::get<1>(v2).allocator_supplied);
}

void allocator_index_constructor_allocator_arg_support(){
    std::cout<<__FUNCTION__<<std::endl;

    se::variant<allocatable,std::string> v1{
        std::allocator_arg_t(),CountingAllocator<int>(),se::in_place<0>};

    assert(allocate_count==0);
    assert(v1.index()==0);
    assert(se::get<0>(v1).allocator_supplied);

    se::variant<std::string,allocatable> v2{
        std::allocator_arg_t(),CountingAllocator<int>(),se::in_place<1>};

    assert(allocate_count==0);
    assert(v2.index()==1);
    assert(se::get<1>(v2).allocator_supplied);
}

void allocator_index_constructor_no_allocator_arg_support(){
    std::cout<<__FUNCTION__<<std::endl;

    se::variant<allocatable_no_arg,std::string> v1{
        std::allocator_arg_t(),CountingAllocator<int>(),se::in_place<0>};

    assert(allocate_count==0);
    assert(v1.index()==0);
    assert(se::get<0>(v1).allocator_supplied);

    se::variant<std::string,allocatable_no_arg> v2{
        std::allocator_arg_t(),CountingAllocator<int>(),se::in_place<1>};

    assert(allocate_count==0);
    assert(v2.index()==1);
    assert(se::get<1>(v2).allocator_supplied);
}

void allocator_type_constructor_no_allocator_support(){
    std::cout<<__FUNCTION__<<std::endl;

    struct MyClass{
        int i;
        MyClass():
            i(42){}
    };

    se::variant<MyClass,std::string> vi{std::allocator_arg_t(),CountingAllocator<MyClass>(),se::in_place<MyClass>};

    assert(allocate_count==0);
    assert(vi.index()==0);
    assert(se::get<0>(vi).i==42);

    se::variant<std::string,MyClass> vi2{std::allocator_arg_t(),CountingAllocator<MyClass>(),se::in_place<MyClass>};

    assert(allocate_count==0);
    assert(vi2.index()==1);
    assert(se::get<1>(vi2).i==42);


    se::variant<not_allocatable,std::string> v1{std::allocator_arg_t(),CountingAllocator<int>(),se::in_place<not_allocatable>};

    assert(allocate_count==0);
    assert(v1.index()==0);
    assert(!se::get<0>(v1).allocator_supplied);

    se::variant<std::string,not_allocatable> v2{std::allocator_arg_t(),CountingAllocator<int>(),se::in_place<not_allocatable>};

    assert(allocate_count==0);
    assert(v2.index()==1);
    assert(!se::get<1>(v2).allocator_supplied);
}

void allocator_type_constructor_allocator_arg_support(){
    std::cout<<__FUNCTION__<<std::endl;

    se::variant<allocatable,std::string> v1{
        std::allocator_arg_t(),CountingAllocator<int>(),se::in_place<allocatable>};

    assert(allocate_count==0);
    assert(v1.index()==0);
    assert(se::get<0>(v1).allocator_supplied);

    se::variant<std::string,allocatable> v2{
        std::allocator_arg_t(),CountingAllocator<int>(),se::in_place<allocatable>};

    assert(allocate_count==0);
    assert(v2.index()==1);
    assert(se::get<1>(v2).allocator_supplied);
}

void allocator_type_constructor_no_allocator_arg_support(){
    std::cout<<__FUNCTION__<<std::endl;

    se::variant<allocatable_no_arg,std::string> v1{
        std::allocator_arg_t(),CountingAllocator<int>(),se::in_place<allocatable_no_arg>};

    assert(allocate_count==0);
    assert(v1.index()==0);
    assert(se::get<0>(v1).allocator_supplied);

    se::variant<std::string,allocatable_no_arg> v2{
        std::allocator_arg_t(),CountingAllocator<int>(),se::in_place<allocatable_no_arg>};

    assert(allocate_count==0);
    assert(v2.index()==1);
    assert(se::get<1>(v2).allocator_supplied);
}

void allocator_copy_constructor_no_allocator_support(){
    std::cout<<__FUNCTION__<<std::endl;

    struct MyClass{
        int i;
        MyClass():
            i(42){}
    };

    se::variant<MyClass,std::string> vis{se::in_place<MyClass>};
    se::variant<MyClass,std::string> vi{std::allocator_arg_t(),CountingAllocator<MyClass>(),vis};

    assert(allocate_count==0);
    assert(vi.index()==0);
    assert(se::get<0>(vi).i==42);

    se::variant<std::string,MyClass> vis2{se::in_place<MyClass>};
    se::variant<std::string,MyClass> vi2{std::allocator_arg_t(),CountingAllocator<MyClass>(),vis2};

    assert(allocate_count==0);
    assert(vi2.index()==1);
    assert(se::get<1>(vi2).i==42);

    se::variant<not_allocatable,std::string> v1s{se::in_place<not_allocatable>};
    se::variant<not_allocatable,std::string> v1{std::allocator_arg_t(),CountingAllocator<int>(),v1s};

    assert(allocate_count==0);
    assert(v1.index()==0);
    assert(!se::get<0>(v1).allocator_supplied);

    se::variant<std::string,not_allocatable> v2s{se::in_place<not_allocatable>};
    se::variant<std::string,not_allocatable> v2{std::allocator_arg_t(),CountingAllocator<int>(),v2s};

    assert(allocate_count==0);
    assert(v2.index()==1);
    assert(!se::get<1>(v2).allocator_supplied);
}

void allocator_copy_constructor_allocator_arg_support(){
    std::cout<<__FUNCTION__<<std::endl;

    se::variant<allocatable,std::string> v1s{se::in_place<allocatable>};
    se::variant<allocatable,std::string> v1{
        std::allocator_arg_t(),CountingAllocator<int>(),v1s};

    assert(allocate_count==0);
    assert(v1.index()==0);
    assert(se::get<0>(v1).allocator_supplied);

    se::variant<std::string,allocatable> v2s{se::in_place<1>};
    se::variant<std::string,allocatable> v2{
        std::allocator_arg_t(),CountingAllocator<int>(),v2s};

    assert(allocate_count==0);
    assert(v2.index()==1);
    assert(se::get<1>(v2).allocator_supplied);
}

void allocator_copy_constructor_no_allocator_arg_support(){
    std::cout<<__FUNCTION__<<std::endl;

    se::variant<allocatable_no_arg,std::string> v1s{se::in_place<allocatable_no_arg>};
    se::variant<allocatable_no_arg,std::string> v1{
        std::allocator_arg_t(),CountingAllocator<int>(),v1s};

    assert(allocate_count==0);
    assert(v1.index()==0);
    assert(se::get<0>(v1).allocator_supplied);

    se::variant<std::string,allocatable_no_arg> v2s{se::in_place<allocatable_no_arg>};
    se::variant<std::string,allocatable_no_arg> v2{
        std::allocator_arg_t(),CountingAllocator<int>(),v2s};

    assert(allocate_count==0);
    assert(v2.index()==1);
    assert(se::get<1>(v2).allocator_supplied);
}

void allocator_move_constructor_no_allocator_support(){
    std::cout<<__FUNCTION__<<std::endl;

    struct MyClass{
        int i;
        bool was_moved=false;
        
        MyClass():
            i(42){}

        MyClass(const MyClass&)=default;
        MyClass(MyClass&&):
            i(42),was_moved(true){}
    };

    se::variant<MyClass,std::string> vis{se::in_place<MyClass>};
    se::variant<MyClass,std::string> vi{std::allocator_arg_t(),CountingAllocator<MyClass>(),std::move(vis)};

    assert(allocate_count==0);
    assert(vi.index()==0);
    assert(se::get<0>(vi).i==42);
    assert(se::get<0>(vi).was_moved);

    se::variant<std::string,MyClass> vis2{se::in_place<MyClass>};
    se::variant<std::string,MyClass> vi2{std::allocator_arg_t(),CountingAllocator<MyClass>(),std::move(vis2)};

    assert(allocate_count==0);
    assert(vi2.index()==1);
    assert(se::get<1>(vi2).i==42);
    assert(se::get<1>(vi2).was_moved);

    se::variant<not_allocatable,std::string> v1s{se::in_place<not_allocatable>};
    se::variant<not_allocatable,std::string> v1{std::allocator_arg_t(),CountingAllocator<int>(),std::move(v1s)};

    assert(allocate_count==0);
    assert(v1.index()==0);
    assert(!se::get<0>(v1).allocator_supplied);
    assert(se::get<0>(v1).was_moved);

    se::variant<std::string,not_allocatable> v2s{se::in_place<not_allocatable>};
    se::variant<std::string,not_allocatable> v2{std::allocator_arg_t(),CountingAllocator<int>(),std::move(v2s)};

    assert(allocate_count==0);
    assert(v2.index()==1);
    assert(!se::get<1>(v2).allocator_supplied);
    assert(se::get<1>(v2).was_moved);
}

void allocator_move_constructor_allocator_arg_support(){
    std::cout<<__FUNCTION__<<std::endl;

    se::variant<allocatable,std::string> v1s{se::in_place<allocatable>};
    se::variant<allocatable,std::string> v1{
        std::allocator_arg_t(),CountingAllocator<int>(),std::move(v1s)};

    assert(allocate_count==0);
    assert(v1.index()==0);
    assert(se::get<0>(v1).allocator_supplied);
    assert(se::get<0>(v1).was_moved);

    se::variant<std::string,allocatable> v2s{se::in_place<1>};
    se::variant<std::string,allocatable> v2{
        std::allocator_arg_t(),CountingAllocator<int>(),std::move(v2s)};

    assert(allocate_count==0);
    assert(v2.index()==1);
    assert(se::get<1>(v2).allocator_supplied);
    assert(se::get<1>(v2).was_moved);
}

void allocator_move_constructor_no_allocator_arg_support(){
    std::cout<<__FUNCTION__<<std::endl;

    se::variant<allocatable_no_arg,std::string> v1s{se::in_place<allocatable_no_arg>};
    se::variant<allocatable_no_arg,std::string> v1{
        std::allocator_arg_t(),CountingAllocator<int>(),std::move(v1s)};

    assert(allocate_count==0);
    assert(v1.index()==0);
    assert(se::get<0>(v1).allocator_supplied);
    assert(se::get<0>(v1).was_moved);

    se::variant<std::string,allocatable_no_arg> v2s{se::in_place<allocatable_no_arg>};
    se::variant<std::string,allocatable_no_arg> v2{
        std::allocator_arg_t(),CountingAllocator<int>(),std::move(v2s)};

    assert(allocate_count==0);
    assert(v2.index()==1);
    assert(se::get<1>(v2).allocator_supplied);
    assert(se::get<1>(v2).was_moved);
}

int main(){
    initial_is_first_type();
    can_construct_first_type();
    can_get_value_of_first_type();
    can_construct_second_type();
    can_move_variant();
    can_copy_variant();
    can_copy_const_variant();
    construction_from_lvalue();
    construction_from_const_lvalue();
    move_construction_with_move_only_types();
    copy_assignment_same_type();
    copy_assignment_to_empty();
    copy_assignment_of_diff_types_destroys_old();
    copy_assignment_from_empty();
    throwing_copy_assign_leaves_target_unchanged();
    move_assignment_to_empty();
    move_assignment_same_type();
    move_assignment_of_diff_types_destroys_old();
    move_assignment_from_empty();
    emplace_construction_by_type();
    emplace_construction_by_index();
    holds_alternative_for_empty_variant();
    holds_alternative_for_non_empty_variant();
    assignment_from_value_to_empty();
    assignment_from_value_to_same_type();
    assignment_from_value_of_diff_types_destroys_old();
    emplace_from_value_to_empty();
    emplace_from_value_to_same_type();
    emplace_from_value_of_diff_types_destroys_old();
    emplace_by_index_to_empty();
    emplace_by_index_to_same_type();
    emplace_by_index_of_diff_types_destroys_old();
    swap_same_type();
    swap_different_types();
    assign_empty_to_empty();
    swap_empties();
    visit();
    reference_members();
    equality();
    less_than();
    constexpr_variant();
    multivisitor();
    sizes();
    duplicate_types();
    non_movable_types();
    direct_init_reference_member();
    reference_types_preferred_for_lvalue();
    construction_with_conversion();
    assignment_with_conversion();
    visitor_with_non_void_return();
    multi_visitor_with_non_void_return();
    initialization_with_initializer_list();
    json();
    nothrow_assign_to_variant_holding_type_with_throwing_move_ok();
    maybe_throw_assign_to_variant_holding_type_with_throwing_move_ok();
    throwing_assign_from_type_leaves_variant_unchanged();
    can_emplace_nonmovable_type_when_other_nothrow_movable();
    throwing_emplace_from_nonmovable_type_leaves_variant_empty();
    throwing_emplace_when_stored_type_can_throw_leaves_variant_empty();
    after_assignment_which_triggers_backup_storage_can_assign_variant();
    backup_storage_and_local_backup();
    large_noexcept_movable_and_small_throw_movable();
    construct_small_with_large_throwables();
    if_emplace_throws_variant_is_valueless();
#ifndef __MINGW32__
    properties();
#endif
    variant_of_references();
    variant_size();
    variant_alternative();
    npos();
    holds_alternative();
    get_with_rvalues();
    get_with_const_rvalues();
    get_if();
    constexpr_comparisons();
    constexpr_visit();
    variant_with_no_types();
    monostate();
    hash();
    allocator_default_constructor_no_allocator_support();
    allocator_default_constructor_allocator_arg_support();
    allocator_default_constructor_no_allocator_arg_support();
    variant_uses_allocator();
    allocator_index_constructor_no_allocator_support();
    allocator_index_constructor_allocator_arg_support();
    allocator_index_constructor_no_allocator_arg_support();
    allocator_type_constructor_no_allocator_support();
    allocator_type_constructor_allocator_arg_support();
    allocator_type_constructor_no_allocator_arg_support();
    allocator_copy_constructor_no_allocator_support();
    allocator_copy_constructor_allocator_arg_support();
    allocator_copy_constructor_no_allocator_arg_support();
    allocator_move_constructor_no_allocator_support();
    allocator_move_constructor_allocator_arg_support();
    allocator_move_constructor_no_allocator_arg_support();
}
