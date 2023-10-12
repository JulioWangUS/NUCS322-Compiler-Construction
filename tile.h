#pragma once

#include <L3.h>

namespace L3{

  void MaximalMunch(Program &p);

  class Tile1_EncDec: public Tile {
        public:
        Tile1_EncDec(){};

        Pattern* try_to_cover(Tree *i, std::vector<Tile*>& all) override;
        std::string printer() override;
  };

  class Tile1_AsmtInTree: public Tile {
        public:
        Tile1_AsmtInTree(){};

        Pattern* try_to_cover(Tree *i, std::vector<Tile*>& all) override;
        std::string printer() override;
  };

  class Tile1_SameLeftVarInTree: public Tile {
        public:
        Tile1_SameLeftVarInTree(){};

        Pattern* try_to_cover(Tree *i, std::vector<Tile*>& all) override;
        std::string printer() override;
  };

  class Tile1_IniMult: public Tile {
        public:
        Tile1_IniMult(){};

        Pattern* try_to_cover(Tree *i, std::vector<Tile*>& all) override;
        std::string printer() override;
  };

  class Tile1_ConsecMultn: public Tile {
        public:
        Tile1_ConsecMultn(){};

        Pattern* try_to_cover(Tree *i, std::vector<Tile*>& all) override;
        std::string printer() override;
  };

  class Tile1_Addn : public Tile {
      public:
      Tile1_Addn(){};
  
      Pattern* try_to_cover(Tree *i, std::vector<Tile*>& all) override;
      std::string printer() override;
  };

  class Tile1_Add : public Tile {
      public:
      Tile1_Add(){};
      
      Pattern* try_to_cover(Tree *i, std::vector<Tile*>& all) override;
      std::string printer() override;
  };

  class Tile2_Lea : public Tile {
    public:
      Tile2_Lea(){};

      Tile2_Lea(Item* w1, Item* w2, Item* w3, int64_t E);
      Item* w1;
	  Item* w2;
	  Item* w3;
	  int64_t E;

      Pattern* try_to_cover(Tree *i, std::vector<Tile*>& all) override;
      std::string printer() override;
  };

  class Tile2_Cjump : public Tile {
    public :
      Tile2_Cjump(){};

      Tile2_Cjump(Item* t1, Op cmp, Item* t2, Item* label);
      Item* label;
	  Item* t1;
      Item* t2;
      Op cmp;

      Pattern* try_to_cover(Tree *i, std::vector<Tile*>& all) override;
      std::string printer() override;
  };

  class Tile2_LoadM : public Tile {
    public:
      Tile2_LoadM(){};

      Tile2_LoadM(Item* w, Item* x, int64_t M);
      Item* w;
	  Item* x;
	  int64_t M;

      Pattern* try_to_cover(Tree *i, std::vector<Tile*>& all) override;
      std::string printer() override;

  };


  class Tile2_SroreM : public Tile {
    public:
      Tile2_SroreM(){};

      Tile2_SroreM(Item* x, int64_t M, Item* s);
      Item* x;
	  int64_t M;
	  Item* s;
  
      Pattern* try_to_cover(Tree *i, std::vector<Tile*>& all) override;
      std::string printer() override;
  };
  
  class Tile3_PP : public Tile {
    public :
      Tile3_PP(){};

      Tile3_PP(Item* w, Op op); 
      Item* w;
      Op op;
      
      Pattern* try_to_cover(Tree *i, std::vector<Tile*>& all) override;
      std::string printer() override;
  };

  class Tile3_SelfOp : public Tile {
    public :
      Tile3_SelfOp(){};

      Tile3_SelfOp(Item* w, Op op, Item* t);
      Item* w;
	  Op op;
	  Item* t;

      Pattern* try_to_cover(Tree *i, std::vector<Tile*>& all) override;
      std::string printer() override;
  };

  class Tile4_AopSop : public Tile {
    public :
      Tile4_AopSop(){};

      Tile4_AopSop(Item* w, Item* t1, Item* t2, Op op);
      int64_t root;
	  Op op;
	  Item* w;
	  Item* t1;
	  Item* t2;

      Pattern* try_to_cover(Tree *i, std::vector<Tile*>& all) override;
      std::string printer() override;
  };

  class Tile4_Asmt : public Tile {
    public :
      Tile4_Asmt(){};

      Tile4_Asmt(Item* w, Item* s);
      Item* w;
      Item* s;

      Pattern* try_to_cover(Tree *i, std::vector<Tile*>& all) override;
      std::string printer() override;
  };

  class Tile4_Cmp : public Tile {
    public :
      Tile4_Cmp(){};

      Tile4_Cmp(Item* w, Item* t1, Item* t2, Op cmp);
      int64_t root;
	  Op cmp;
	  Item* w;
	  Item* t1;
	  Item* t2;

      Pattern* try_to_cover(Tree *i, std::vector<Tile*>& all) override;
      std::string printer() override;
  };

  class Tile4_Ret : public Tile {
    public :
      Tile4_Ret(){};

      Tile4_Ret(Item* t);
      Item* t;

      Pattern* try_to_cover(Tree *i, std::vector<Tile*>& all) override;
      std::string printer() override;
  };

  class Tile4_Label : public Tile {
    public :
      Tile4_Label(){};

      Tile4_Label(Item* label, Op op);
      Item* label;
      Op op;

      Pattern* try_to_cover(Tree *i, std::vector<Tile*>& all) override;
      std::string printer() override;
  };

  class Tile4_Call : public Tile {
    public :
      Tile4_Call(){};

      Tile4_Call(Tree* t);
      Tree* t;

      Pattern* try_to_cover(Tree *i, std::vector<Tile*>& all) override;
      std::string printer() override; 
  };

}