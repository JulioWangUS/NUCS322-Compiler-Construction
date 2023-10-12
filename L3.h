#pragma once

#include <array>
#include <vector>
#include <string>
#include <unordered_set>
#include <map>
#include <set>


namespace L3 {
  class Visitor;
  class Itemprinter;

  enum Op {add, addn, sub, subn, mult, multn, band, bandn, s_l, s_ln, s_r, s_rn, c_l, c_le, c_e, c_g, c_ge, asmt, load, store, ret, cjmp, br, label, call, leaf};
  enum ItemType {VAR, NUM, LABEL, FUN};

  /*
   * items for tree-node construction
   */
  class Item {
    public:
      Item (ItemType n);
      virtual int64_t getval();

      ItemType type;
  };

  class Var : public Item {
    public:
      Var (int n);
      int64_t getval() override;

      int var;  // vars are encoded as interger in the parser
  };

  class Num : public Item {
    public:
      Num (int64_t n);
      int64_t getval() override;

      int64_t num;
  };

  class Label : public Item {
    public:
      Label (int n);
      int64_t getval() override;

      int label;
  };

  class FunName : public Item {
    public:
      FunName (std::string s);

      std::string fun;
  };


  /*
   * tree node
   */
   class Tree;
   class Pattern;

   class Tile {
     public :
       virtual Pattern* try_to_cover(Tree *i, std::vector<Tile*>& all) = 0;
       virtual std::string printer() = 0;
   };

   class Tree { //Tree of instructions
     public:
       Tree (Item* i, Op o);
       friend class Tree;

       Item* root;
       Op op;
       int id_in_func;
       std::vector<Tree*> leaves;
   };

   class Pattern { //Tree of generated pattern
     public:
       Pattern (Tile* t);
       friend class Pattern;

       Tile* tile;
       std::vector<Pattern*> leaves;
   };

  class Context{
    public:
      std::vector<Tree *> trees;
      std::vector<Pattern *> patterns;
  };
  
  /*
   * Function.
   */
  class Function{
    public:
      std::string name;
      std::vector<Item *> args;

      std::vector<Context *> contexts;

      std::map<std::string, int> label_map; // label encoding
      std::map<std::string, int> var_map;   // var encoding
      std::map<int, int> label_id_map;      // for liveness analysis
      int var_count;
      int tree_count;
  };

  class Program{
    public:
      std::vector<Function *> functions;
      std::string entryPointLabel;
      int global_label_count;
  };

  
}