#include <sched.h>
#include <string>
#include <vector>
#include <utility>
#include <algorithm>
#include <set>
#include <iterator>
#include <cstring>
#include <cctype>
#include <cstdlib>
#include <stdint.h>
#include <assert.h>

#include <tao/pegtl.hpp>
#include <tao/pegtl/analyze.hpp>
#include <tao/pegtl/contrib/raw_string.hpp>


#include <L3.h>
#include <L3parser.h>

namespace pegtl = tao::TAO_PEGTL_NAMESPACE;

using namespace pegtl;

namespace L3 {

  /* 
   * Tokens parsed
   */ 
  std::vector<Item *> parsed_items;
  std::vector<Op> parsed_ops;

  /* 
   * Grammar rules from now on.
   */

  /* 
   * Keywords.
   */
  struct str_return : TAOCPP_PEGTL_STRING( "return" ) {};
  struct str_call : TAOCPP_PEGTL_STRING( "call" ) {};
  struct str_print : TAOCPP_PEGTL_STRING( "print" ) {};
  struct str_input : TAOCPP_PEGTL_STRING( "input" ) {};
  struct str_allocate : TAOCPP_PEGTL_STRING( "allocate" ) {};
  struct str_error : TAOCPP_PEGTL_STRING( "tensor-error" ) {};
  struct str_br : TAOCPP_PEGTL_STRING( "br " ) {};
  struct str_load : TAOCPP_PEGTL_STRING( "load " ) {};
  struct str_store : TAOCPP_PEGTL_STRING( "store " ) {};
  struct str_define : TAOCPP_PEGTL_STRING( "define " ) {};

  struct str_arrow : TAOCPP_PEGTL_STRING( "<-" ) {};
  struct str_sop_l : TAOCPP_PEGTL_STRING( "<<" ) {};
  struct str_sop_r : TAOCPP_PEGTL_STRING( ">>" ) {};
  struct str_cmp_le : TAOCPP_PEGTL_STRING( "<=" ) {};
  struct str_cmp_ge : TAOCPP_PEGTL_STRING( ">=" ) {};
  struct str_cmp_l : TAOCPP_PEGTL_STRING( "<" ) {};
  struct str_cmp_e : TAOCPP_PEGTL_STRING( "=" ) {};
  struct str_cmp_g : TAOCPP_PEGTL_STRING( ">" ) {};
  struct str_aop_p : TAOCPP_PEGTL_STRING( "+" ) {};
  struct str_aop_s : TAOCPP_PEGTL_STRING( "-" ) {};
  struct str_aop_m : TAOCPP_PEGTL_STRING( "*" ) {};
  struct str_aop_a : TAOCPP_PEGTL_STRING( "&" ) {};

  struct number:
    pegtl::seq<
      pegtl::opt<
        pegtl::sor<
          pegtl::one< '-' >,
          pegtl::one< '+' >
        >
      >,
      pegtl::plus< 
        pegtl::digit
      >
    >{};

  struct name:
    pegtl::seq<
      pegtl::plus< 
        pegtl::sor<
          pegtl::alpha,
          pegtl::one< '_' >
        >
      >,
      pegtl::star<
        pegtl::sor<
          pegtl::alpha,
          pegtl::one< '_' >,
          pegtl::digit
        >
      >
    > {};

  struct var:
    pegtl::seq<
      pegtl::one<'%'>,
      name
    > {};

  struct label:
    pegtl::seq<
      pegtl::one<':'>,
      name
    > {};

  struct function_name_rule:
    pegtl::seq<
      pegtl::one<'@'>,
      name
    > {};

  struct instruction_function_name_rule:
    pegtl::seq<
      pegtl::one<'@'>,
      name
    > {};

  struct item_label:
    label {};

  struct u_rule:
    pegtl::sor<
      var,
      instruction_function_name_rule
    > {};

  struct t_rule:
    pegtl::sor<
      var,
      number
    > {};

  struct s_rule:
    pegtl::sor<
      t_rule,
      item_label,
      instruction_function_name_rule
    > {};

  struct oop_rule:
      pegtl::sor<
      str_cmp_le,
      str_cmp_l,
      str_cmp_e,
      str_cmp_ge,
      str_cmp_g,
      str_aop_p,
      str_aop_s,
      str_aop_m,
      str_aop_a
    > {};

    struct sop_rule:
      pegtl::sor<
      str_sop_l,
      str_sop_r
    > {};

    struct op_rule:  //must be separated
      pegtl::sor<
      sop_rule,
      oop_rule
    > {};

  struct comment: 
    pegtl::disable< 
      TAOCPP_PEGTL_STRING( "//" ), 
      pegtl::until< pegtl::eolf > 
    > {};

  struct seps: 
    pegtl::star< 
      pegtl::sor< 
        pegtl::ascii::space, 
        comment 
      > 
    > {};

  struct vars_rule:
    pegtl::opt<
        pegtl::seq<
          var,
          pegtl::star<
            pegtl::seq<
            seps,
            pegtl::one< ',' >,
            seps,
            var,
            seps
            >
          >
        >
    >{};

  struct args_rule:
    pegtl::opt<
        pegtl::seq<
          t_rule,
          pegtl::star<
            pegtl::seq<
            seps,
            pegtl::one< ',' >,
            seps,
            t_rule,
            seps
            >
          >
        >
    >{};

  struct callee_rule:
    pegtl::sor<
      u_rule,
      str_print,
      str_input,
      str_allocate,
      str_error
    > {};

  struct call_rule:
    pegtl::seq<
      str_call,
      seps,
      callee_rule,
      seps,
      pegtl::one< '(' >,
      seps,
      args_rule,
      seps,
      pegtl::one< ')' >
    > {};

  struct call_item:
    call_rule {};

  struct Instruction_call_rule:
    call_rule {};

  struct Instruction_call_assignment_rule:
    pegtl::seq<
      var,
      seps,
      str_arrow,
      seps,
      call_item
    > {};

  struct Instruction_label_rule:
    label {};

  struct Instruction_jump_rule:
    pegtl::seq<
      str_br,
      seps,
      t_rule,
      seps,
      item_label
    > {};

  struct Instruction_goto_rule:
    pegtl::seq<
      str_br,
      seps,
      item_label
    > {};

  struct Instruction_return_rule:
    pegtl::seq<
      str_return
    > { };

  struct Instruction_return_t_rule:
    pegtl::seq<
      str_return,
      seps,
      t_rule
    > { };

  struct Instruction_op_assignment_rule:
    pegtl::seq<
      var,
      seps,
      str_arrow,
      seps,
      t_rule,
      seps,
      op_rule,
      seps,
      t_rule
    > {};

  struct Instruction_simple_assignment_rule:
    pegtl::seq<
      var,
      seps,
      str_arrow,
      seps,
      s_rule
    > {};

  struct Instruction_load_rule:
    pegtl::seq<
      var,
      seps,
      str_arrow,
      seps,
      str_load,
      seps,
      var
    > {};

  struct Instruction_store_rule:
    pegtl::seq<
      seps,  
      str_store,
      seps,
      var,
      seps,
      str_arrow,
      seps,
      s_rule
    > {};

  struct Instruction_rule:
    pegtl::sor<
      pegtl::seq< pegtl::at<Instruction_op_assignment_rule>            , Instruction_op_assignment_rule             >,
      pegtl::seq< pegtl::at<Instruction_load_rule>        , Instruction_load_rule         >,
      pegtl::seq< pegtl::at<Instruction_store_rule>        , Instruction_store_rule         >,
      pegtl::seq< pegtl::at<Instruction_simple_assignment_rule>        , Instruction_simple_assignment_rule         >,
      pegtl::seq< pegtl::at<Instruction_call_assignment_rule>        , Instruction_call_assignment_rule         >,
      pegtl::seq< pegtl::at<Instruction_call_rule>        , Instruction_call_rule         >,
      pegtl::seq< pegtl::at<Instruction_jump_rule>        , Instruction_jump_rule         >,
      pegtl::seq< pegtl::at<Instruction_goto_rule>        , Instruction_goto_rule         >,
      pegtl::seq< pegtl::at<Instruction_label_rule>        , Instruction_label_rule         >,
      pegtl::seq< pegtl::at<Instruction_return_t_rule>        , Instruction_return_t_rule         >,
      pegtl::seq< pegtl::at<Instruction_return_rule>        , Instruction_return_rule         >
    > { };

  struct Instructions_rule:
    pegtl::plus<
      pegtl::seq<
        seps,
        Instruction_rule,
        seps
      >
    > { };

  struct Function_rule:
    pegtl::seq<
      seps,
      str_define,
      seps,
      function_name_rule,
      seps,
      pegtl::one< '(' >,
      seps,
      vars_rule,
      seps,
      pegtl::one< ')' >,
      seps,
      pegtl::one< '{' >,
      seps,
      Instructions_rule,
      seps,
      pegtl::one< '}' >,
      seps
    > {};

  struct Functions_rule:
    pegtl::plus<
      seps,
      Function_rule,
      seps
    > {};

  struct grammar : 
    pegtl::must< 
      Functions_rule
    > {};

  /* 
   * Actions attached to grammar rules.
   */
  template< typename Rule >
  struct action : pegtl::nothing< Rule > {};

  template<> struct action < function_name_rule > {
    template< typename Input >
	  static void apply( const Input & in, Program & p){
       //if (p.entryPointLabel.empty()){
       //    p.entryPointLabel = in.string();
       //    p.global_label_count = 0;
       //}
        auto newF = new Function();
        newF->name = in.string();
        newF->var_count = 0;
        newF->tree_count = 0;
        auto newC = new Context();
        newF->contexts.push_back(newC);
        p.functions.push_back(newF);
    }
  };

  template<> struct action < vars_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
      auto currentF = p.functions.back();
      currentF->args = parsed_items;
      parsed_items = std::vector<Item *>();
    }
  };

  template<> struct action < var > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
      auto currentF = p.functions.back();
      std::string s = in.string();
      std::map<std::string, int>::iterator it;     
      it = currentF->var_map.find(s);

      int n = 0;
      if (it != currentF->var_map.end()) {
          n = it->second;
      } else {
          n = ++(currentF->var_count);
          currentF->var_map.insert(std::pair<std::string, int>(s, n));
      }

      auto v = new Var(n);
      parsed_items.push_back(v);
    }
  };

  template<> struct action < number > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
      auto n = new Num(std::stoll(in.string()));
      parsed_items.push_back(n);
    }
  };

  template<> struct action < item_label > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
      auto currentF = p.functions.back();
      std::string s = in.string();
      std::map<std::string, int>::iterator it;     
      it = currentF->label_map.find(s);

      int n = 0;
      if (it != currentF->label_map.end()) {
          n = it->second;
      } else {
          n = ++(p.global_label_count);
          currentF->label_map.insert(std::pair<std::string, int>(s, n));
      }

      auto l = new Label(n);
      parsed_items.push_back(l);
    }
  };

  template<> struct action < instruction_function_name_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
      auto n = new FunName(in.string());
      parsed_items.push_back(n);
    }
  };

  template<> struct action < str_print > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
      auto n = new FunName(in.string());
      parsed_items.push_back(n);
    }
  };

  template<> struct action < str_input > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
      auto n = new FunName(in.string());
      parsed_items.push_back(n);
    }
  };

  template<> struct action < str_allocate > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
      auto n = new FunName(in.string());
      parsed_items.push_back(n);
    }
  };

  template<> struct action < str_error > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
      auto n = new FunName(in.string());
      parsed_items.push_back(n);
    }
  };

  template<> struct action < str_cmp_le > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
      parsed_ops.push_back(Op::c_le);
    }
  };

  template<> struct action < str_cmp_l > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
      parsed_ops.push_back(Op::c_l);
    }
  };

  template<> struct action < str_cmp_e > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
      parsed_ops.push_back(Op::c_e);
    }
  };

  template<> struct action < str_cmp_g > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
      parsed_ops.push_back(Op::c_g);
    }
  };

  template<> struct action < str_cmp_ge > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
      parsed_ops.push_back(Op::c_ge);
    }
  };

  template<> struct action < str_sop_l > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
      parsed_ops.push_back(Op::s_l);
    }
  };

  template<> struct action < str_sop_r > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
      parsed_ops.push_back(Op::s_r);
    }
  };

  template<> struct action < str_aop_p > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
      parsed_ops.push_back(Op::add);
    }
  };

  template<> struct action < str_aop_s > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
      parsed_ops.push_back(Op::sub);
    }
  };

  template<> struct action < str_aop_m > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
      parsed_ops.push_back(Op::mult);
    }
  };

  template<> struct action < str_aop_a > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
      parsed_ops.push_back(Op::band);
    }
  };

  int64_t arithmetic(int64_t l, int64_t r, Op op) {
      switch(op) {
          case Op::add :
          return l + r;
          case Op::sub :
          return l - r;
          case Op::mult :
          return l * r;
          case Op::band :
          return l & r;
          case Op::s_l :
          return l << r;
          case Op::s_r :
          return l >> r;
      }
      assert(0);
      return 0;
  }

  template<> struct action < Instruction_op_assignment_rule > {
    template< typename Input >
	  static void apply( const Input & in, Program & p){
      auto currentF = p.functions.back();
      auto currentC = currentF->contexts.back();

      auto op = parsed_ops.back();
      parsed_ops.pop_back();
      auto right = parsed_items.back();
      parsed_items.pop_back();
      auto left = parsed_items.back();
      parsed_items.pop_back();
      auto root = parsed_items.back();
      parsed_items.pop_back();

      bool ifSwap = false;  // if the left and the right should be switched
      if (op == Op::c_g || op == Op::c_ge) {
          ifSwap = true;
          if(op == Op::c_g) {
              op = Op::c_l;
          } else {
              op = Op::c_le;
          }
      } else if (op == Op::c_l || op == Op::c_le || op == Op::c_e){
          ifSwap = false;
      } else if (right->type == NUM && left->type == NUM) {  // both constants, turned to a simple assignment
          int64_t n = arithmetic(left->getval(), right->getval(), op);
          auto num = new Num(n);
          auto lf = new Tree(num, Op::leaf);
          auto t = new Tree(root, Op::asmt);
          t->leaves.push_back(lf);
          t->id_in_func = (currentF->tree_count)++;
          currentC->trees.push_back(t);
          return;
      } else if (op == Op::add || op == Op::mult || op == Op::band) {
          if(left->type == NUM || right->type == root->type && right->getval() == root->getval()) {  // no NUM on the left, same-name var must be on the left
              ifSwap = true;
          }
      } else if (right->type == root->type && right->getval() == root->getval()) {  // [v <- 3 - v]
          int n = ++(currentF->var_count);
          auto v = new Var(n);
          auto a = new Tree(v, Op::asmt);
          auto lf = new Tree(root, Op::leaf);
          a->leaves.push_back(lf);
          a->id_in_func = (currentF->tree_count)++;
          currentC->trees.push_back(a);

          auto l = new Tree(left, Op::leaf);
          auto r = new Tree(v, Op::leaf);
          auto t = new Tree(root, op);
          t->leaves.push_back(l);
          t->leaves.push_back(r);
          t->id_in_func = (currentF->tree_count)++;
          currentC->trees.push_back(t);
          return;
      }

      auto l = new Tree(left, Op::leaf);
      auto r = new Tree(right, Op::leaf);
      auto t = new Tree(root, op);
      if (ifSwap) {
          auto temp = l;
          l = r;
          r = temp;
      }
      if (r->root->type == NUM && t->op < c_l) {  //distinguish [v + v] / [v + c]
          int temp;
          temp = t->op;
          temp++;
          t->op = static_cast<Op>(temp);
      }
      if (t->op == Op::addn && r->root->getval() == 0 && t->root->getval() == l->root->getval()) { return; }
      if (t->op == Op::multn && r->root->getval() == 1 && t->root->getval() == l->root->getval()) { return; }
      if (t->op == Op::multn && r->root->getval() == 2) { t->op = Op::s_ln; r->root = new Num(1); }
      if (t->op == Op::multn && r->root->getval() == 4) { t->op = Op::s_ln; r->root = new Num(2); }
      if (t->op == Op::multn && r->root->getval() == 8) { t->op = Op::s_ln; r->root = new Num(3); }

      t->leaves.push_back(l);
      t->leaves.push_back(r);
      
      t->id_in_func = (currentF->tree_count)++;
      currentC->trees.push_back(t);
    }
  };

  template<> struct action < Instruction_simple_assignment_rule > {
    template< typename Input >
	  static void apply( const Input & in, Program & p){
      auto currentF = p.functions.back();
      auto currentC = currentF->contexts.back();

      auto lf = parsed_items.back();
      parsed_items.pop_back();
      auto root = parsed_items.back();
      parsed_items.pop_back();

      auto l = new Tree(lf, Op::leaf);
      auto t = new Tree(root, Op::asmt);
      t->leaves.push_back(l);
      t->id_in_func = (currentF->tree_count)++;
      currentC->trees.push_back(t);
    }
  };

  template<> struct action < Instruction_load_rule > {
    template< typename Input >
	  static void apply( const Input & in, Program & p){
      auto currentF = p.functions.back();
      auto currentC = currentF->contexts.back();

      auto lf = parsed_items.back();
      parsed_items.pop_back();
      auto root = parsed_items.back();
      parsed_items.pop_back();

      auto l = new Tree(lf, Op::leaf);
      auto t = new Tree(root, Op::load);
      t->leaves.push_back(l);
      t->id_in_func = (currentF->tree_count)++;
      currentC->trees.push_back(t);
    }
  };

  template<> struct action < Instruction_store_rule > {
    template< typename Input >
	  static void apply( const Input & in, Program & p){
      auto currentF = p.functions.back();
      auto currentC = currentF->contexts.back();

      auto right = parsed_items.back();
      parsed_items.pop_back();
      auto left = parsed_items.back();
      parsed_items.pop_back();

      auto l = new Tree(left, Op::leaf);
      auto r = new Tree(right, Op::leaf);
      auto t = new Tree(NULL, Op::store);
      t->leaves.push_back(l);
      t->leaves.push_back(r);
      t->id_in_func = (currentF->tree_count)++;
      currentC->trees.push_back(t);
    }
  };
  
  template<> struct action < Instruction_return_rule > {
    template< typename Input >
	  static void apply( const Input & in, Program & p){
      auto currentF = p.functions.back();
      auto currentC = currentF->contexts.back(); 

      auto t = new Tree(NULL, Op::ret);
      t->id_in_func = (currentF->tree_count)++;
      currentC->trees.push_back(t);

      auto newC = new Context();
      currentF->contexts.push_back(newC);
    }
  };

  template<> struct action < Instruction_return_t_rule > {
    template< typename Input >
	  static void apply( const Input & in, Program & p){
      auto currentF = p.functions.back();
      auto currentC = currentF->contexts.back();

      auto lf = parsed_items.back();
      parsed_items.pop_back();

      auto t = new Tree(NULL, Op::ret);
      auto l = new Tree(lf, Op::leaf);
      t->leaves.push_back(l);
      t->id_in_func = (currentF->tree_count)++;
      currentC->trees.push_back(t);

      auto newC = new Context();
      currentF->contexts.push_back(newC);
    }
  };
  

  template<> struct action < Instruction_jump_rule > {
    template< typename Input >
	  static void apply( const Input & in, Program & p){
      auto currentF = p.functions.back();
      auto currentC = currentF->contexts.back();

      auto label = parsed_items.back();
      parsed_items.pop_back();
      auto cond = parsed_items.back();
      parsed_items.pop_back();

      if(cond->type == ItemType::NUM) {
          if(cond->getval() != 0) {  // goto
              auto t = new Tree(label, Op::br);
              t->id_in_func = (currentF->tree_count)++;
              currentC->trees.push_back(t);

              auto newC = new Context();
              currentF->contexts.push_back(newC);
          }
          return;
      }

      auto t = new Tree(label, Op::cjmp);
      auto l = new Tree(cond, Op::leaf);
      t->leaves.push_back(l);
      t->id_in_func = (currentF->tree_count)++;
      currentC->trees.push_back(t);

      auto newC = new Context();
      currentF->contexts.push_back(newC);
    }
  };

  template<> struct action < Instruction_goto_rule > {
    template< typename Input >
	  static void apply( const Input & in, Program & p){
      auto currentF = p.functions.back();
      auto currentC = currentF->contexts.back();

      auto label = parsed_items.back();
      parsed_items.pop_back();

      auto t = new Tree(label, Op::br);
      t->id_in_func = (currentF->tree_count)++;
      currentC->trees.push_back(t);

      auto newC = new Context();
      currentF->contexts.push_back(newC);
    }
  };

  template<> struct action < Instruction_label_rule > {
    template< typename Input >
	  static void apply( const Input & in, Program & p){
      auto currentF = p.functions.back();
      std::string s = in.string();
      std::map<std::string, int>::iterator it;     
      it = currentF->label_map.find(s);

      int n = 0;
      if (it != currentF->label_map.end()) {
          n = it->second;
      } else {
          n = ++(p.global_label_count);
          currentF->label_map.insert(std::pair<std::string, int>(s, n));
      }

      auto l = new Label(n);
      auto t = new Tree(l, Op::label);
      t->id_in_func = (currentF->tree_count)++;
      currentF->label_id_map.insert(std::pair<int, int>(n, t->id_in_func));

      auto currentC = currentF->contexts.back();
      if(currentC->trees.empty()) {
          currentC->trees.push_back(t);
      } else {
          auto newC = new Context();
          newC->trees.push_back(t);
          currentF->contexts.push_back(newC);
      }
      
      auto newC1 = new Context();
      currentF->contexts.push_back(newC1);
    }
  };

  template<> struct action < Instruction_call_rule > {
    template< typename Input >
	  static void apply( const Input & in, Program & p){
      auto currentF = p.functions.back();

      auto callee = parsed_items.front();
      parsed_items.erase (parsed_items.begin());
      auto args = parsed_items;
      parsed_items = std::vector<Item *>();

      int n = ++(p.global_label_count);
      auto l = new Label(n);
      auto t = new Tree(NULL, Op::call);
      
      for (auto i : args) {
          auto leaf = new Tree(i, Op::leaf);
          t->leaves.push_back(leaf);
      }
      auto leaf = new Tree(l, Op::leaf);
      t->leaves.push_back(leaf);
      leaf = new Tree(callee, Op::leaf);
      t->leaves.push_back(leaf);
      t->id_in_func = (currentF->tree_count)++;

      auto currentC = currentF->contexts.back();
      if(currentC->trees.empty()) {
          currentC->trees.push_back(t);
      } else {
          auto newC = new Context();
          newC->trees.push_back(t);
          currentF->contexts.push_back(newC);
      }

      auto newC1 = new Context();
      currentF->contexts.push_back(newC1);
    }
  };

  template<> struct action < Instruction_call_assignment_rule > {
    template< typename Input >
      static void apply( const Input & in, Program & p){
	  auto currentF = p.functions.back();

      auto dst =  parsed_items.front();
      parsed_items.erase (parsed_items.begin());
      auto callee = parsed_items.front();
      parsed_items.erase (parsed_items.begin());
      auto args = parsed_items;
      parsed_items = std::vector<Item *>();

      int n = ++(p.global_label_count);
      auto l = new Label(n);
      auto t = new Tree(dst, Op::call);
      
      for (auto i : args) {
          auto leaf = new Tree(i, Op::leaf);
          t->leaves.push_back(leaf);
      }
      auto leaf = new Tree(l, Op::leaf);
      t->leaves.push_back(leaf);
      leaf = new Tree(callee, Op::leaf);
      t->leaves.push_back(leaf);
      t->id_in_func = (currentF->tree_count)++;

      auto currentC = currentF->contexts.back();
      if(currentC->trees.empty()) {
          currentC->trees.push_back(t);
      } else {
          auto newC = new Context();
          newC->trees.push_back(t);
          currentF->contexts.push_back(newC);
      }

      auto newC1 = new Context();
      currentF->contexts.push_back(newC1);
    }
  };

  

  Program ParseFile (char *fileName){

    /* 
     * Check the grammar for some possible issues.
     */
    pegtl::analyze< grammar >();

    /*
     * Parse.
     */
    file_input< > fileInput(fileName);
    Program p;
    p.entryPointLabel = "@main";
    p.global_label_count = 0;
    parse< grammar, action >(fileInput, p);
    for(auto f : p.functions) {
        f->contexts.pop_back();
    }
    return p;
  }

}
