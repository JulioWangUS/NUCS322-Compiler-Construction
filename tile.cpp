#include <string>
#include <iostream>
#include <fstream>
#include <cassert>
#include <vector>



#include <tile.h>

namespace L3 {
    void TreeSimplifier(std::vector<Tile*>& all);
	void PatternGenerator(std::vector<Tile*>& all);
	Pattern* Cover(Tree* i, std::vector<Tile*>& all);

	void MaximalMunch(Program &p) {
		std::vector<Tile*> pre;
	    std::vector<Tile*> all;
		TreeSimplifier(pre);
		PatternGenerator(all);                  // hard encoded maximum rule guarantee
		
		for(auto f : p.functions) {
			for(auto c : f->contexts) {
				for(auto i : c->trees) {
					Cover(i, pre);              // run all of the methods to simplify the original tree
					auto p = Cover(i, all);     // cover the tree with the optimal method to achieve maximal munch
					c->patterns.push_back(p);
				}
			}
		}
		return;
	}

	Pattern* Cover(Tree* i, std::vector<Tile*>& all) {
		for(auto t : all) {
			auto p = t->try_to_cover(i, all);
			if(p != NULL) {
				return p;
			}
		}
	}

	/*
	 * Vector of tiles initialization, with hard encoded maximal munch guarantee
	 */
	void TreeSimplifier(std::vector<Tile*>& all) {  // traversal with each method to simplify the trees

	    /* specials with knowledge of higher levels */	
	    auto t11 = new Tile1_EncDec;
		all.push_back(t11);
		auto t12 = new Tile1_AsmtInTree;
		all.push_back(t12);
		auto t13 = new Tile1_SameLeftVarInTree;
		all.push_back(t13);
		auto t14 = new Tile1_IniMult;
		all.push_back(t14);
		auto t15 = new Tile1_ConsecMultn;
		all.push_back(t15);
		auto t16 = new Tile1_Addn;
		all.push_back(t16);
		auto t17 = new Tile1_Add;
		all.push_back(t17);
	}

	void PatternGenerator(std::vector<Tile*>& all) {  // optimal method first to cover a tree and generate a pattern
		
	    /* tiles that cover multiiple levels of a tree */
		auto t21 = new Tile2_Lea;
		all.push_back(t21);
		auto t22 = new Tile2_Cjump;
		all.push_back(t22);
		auto t23 = new Tile2_LoadM; 
		all.push_back(t23);
		auto t24 = new Tile2_SroreM;
		all.push_back(t24); 

		/* single level of a tree with better L2 code */
	    auto t31 = new Tile3_PP;
		all.push_back(t31);
		auto t32 = new Tile3_SelfOp;
		all.push_back(t32);

		/* basics that do not overlap */
		auto t41 = new Tile4_AopSop;
		all.push_back(t41);
		auto t42 = new Tile4_Asmt;
		all.push_back(t42);
		auto t43 = new Tile4_Cmp;
		all.push_back(t43);
		auto t44 = new Tile4_Ret;
		all.push_back(t44);
		auto t45 = new Tile4_Label;
		all.push_back(t45);
		auto t46 = new Tile4_Call;
		all.push_back(t46);
	}


	/*
	 * Auxiliary functions.
	 */
	std::string OpPrinter(Op op) {
		switch(op) {
          case Op::add :
          return "+=";
		  case Op::addn :
          return "+=";
          case Op::sub :
          return "-=";
		  case Op::subn :
          return "-=";
          case Op::mult :
          return "*=";
		  case Op::multn :
          return "*=";
          case Op::band :
          return "&=";
		  case Op::bandn :
          return "&=";
          case Op::s_l :
          return "<<=";
		  case Op::s_ln :
          return "<<=";
          case Op::s_r :
          return ">>=";
		  case Op::s_rn :
          return ">>=";
		  case Op::c_le :
		  return "<=";
		  case Op::c_l :
		  return "<";
		  case Op::c_e :
		  return "=";
      }
      assert(0);
      return "ERROR!";
	}

    std::string ItemPrinter(Item* i) {
		std::string s;
		s += " ";
		if (i->type == VAR) {
			s += "%v" + std::to_string(i->getval());
		} else if (i->type == NUM) {
			s += std::to_string(i->getval());
		} else if (i->type == LABEL) {
			s += ":l" + std::to_string(i->getval());
		} else {
			s += dynamic_cast<FunName*>(i)->fun;
		}
		s += " ";
		return s;
	}

	void SubTreeRecursion(Tree* t, Pattern* p, std::vector<Tile*>& howToDo) {
		if (t->op != Op::leaf) {
			p->leaves.push_back(Cover(t, howToDo));
		}
	}

	void LeavesRecursion(Tree* t, std::vector<Tile*>& dummy, Tile* tile) {
		if (t->leaves.size()) {
			for (auto leaf : t->leaves) {
				tile->try_to_cover(leaf, dummy);
			}
		}
	}


	/*
	 * Tiles.
	 */
	std::string Tile1_EncDec::printer() { return ""; }                       // eliminate the contiguous encoding/decoding
	Pattern* Tile1_EncDec::try_to_cover(Tree *i, std::vector<Tile*>& all) {  // [ v <<= 1 ][ v += 1 ][ v >>= 1 ]
		if (i->op == s_rn && i->leaves.back()->root->getval() == 1) {
			auto j = i->leaves.front();
			if (j->op == addn && j->leaves.back()->root->getval() == 1) {
				auto k = j->leaves.front();
				if (k->op == s_ln && k->leaves.back()->root->getval() == 1) {
					i->leaves.front() = k->leaves.front();
					i->leaves.pop_back();
					i->op = Op::asmt;
				}
			}
		}
		LeavesRecursion(i, all, this);
		return NULL;
	}


	std::string Tile1_AsmtInTree::printer() { return ""; }                       // eliminate the assignments inside a tree
	Pattern* Tile1_AsmtInTree::try_to_cover(Tree* i, std::vector<Tile*>& all) {  // [ a <- b ]
		if (i->op == asmt && i->leaves.front()->op != leaf && i->leaves.front()->root->type == VAR) {
			i->op = i->leaves.front()->op;
			i->leaves = i->leaves.front()->leaves;
			try_to_cover(i, all);
		} else {
			LeavesRecursion(i, all, this);
		}
		return NULL;
	}


	std::string Tile1_SameLeftVarInTree::printer() { return ""; }                       // assign a same name to the left vars in a tree as the root 
	Pattern* Tile1_SameLeftVarInTree::try_to_cover(Tree *i, std::vector<Tile*>& all) {
		if (i->op <= s_rn && i->leaves.front()->op != leaf) {
			if (i->root->type != i->leaves.back()->root->type 
			 || i->root->getval() != i->leaves.back()->root->getval()) {
			    if (i->leaves.front()->leaves.size() == 1 
				 || i->leaves.front()->leaves.back()->root->type != i->root->type 
				 || i->leaves.front()->leaves.back()->root->getval() != i->root->getval()) {
					i->leaves.front()->root = i->root;
				}
			}
		}
		LeavesRecursion(i, all, this);
		return NULL;
	}


	std::string Tile1_IniMult::printer() { return ""; }                       // mult following initialization of 1
	Pattern* Tile1_IniMult::try_to_cover(Tree *i, std::vector<Tile*>& all) {
		if (i->op == mult && i->leaves.front()->op == mult) {
			auto j = i->leaves.front()->leaves.front();
			if (j->op == asmt && j->leaves.front()->root->type == NUM && j->leaves.front()->root->getval() == 1) {
				i->leaves.front() = i->leaves.front()->leaves.back();
			}
		}
		LeavesRecursion(i, all, this);
		return NULL;
	}


	std::string Tile1_ConsecMultn::printer() { return ""; }                       // pre-process the consecutive multiplications by constants
	Pattern* Tile1_ConsecMultn::try_to_cover(Tree* i, std::vector<Tile*>& all) {  // [ *|<< ]
		int64_t multiplier = 1;
		int64_t bitwiser = 0;
		auto iterator = i;

		while (iterator->op == Op::multn || iterator->op == Op::s_ln) {
			if (iterator->op == Op::multn) {
				multiplier *= iterator->leaves.back()->root->getval();
			} else {
				bitwiser += iterator->leaves.back()->root->getval();
			}
			iterator = iterator->leaves.front();
		}

		auto numM = new Num(multiplier);
		auto leafM = new Tree(numM, Op::leaf);
		auto numB = new Num(bitwiser);
		auto leafB = new Tree(numB, Op::leaf);

		if (multiplier == 1 && bitwiser > 0) {
			i->leaves.front() = iterator;
			i->leaves.back() = leafB;
		} else if (multiplier > 1 && bitwiser == 0) {
			i->leaves.front() = iterator;
			i->leaves.back() = leafM;
		} else if (multiplier > 1 && bitwiser > 0) {
			i->op = Op::s_ln;
			i->leaves.back() = leafB;
			i->leaves.front()->op = Op::multn;
			i->leaves.front()->leaves.back() = leafM;
			i->leaves.front()->leaves.front() = iterator;
		}

		LeavesRecursion(iterator, all, this);
		return NULL;
	}


	std::string Tile1_Addn::printer() { return ""; }                       // LA::[ a <- b + const ] --> L3::[ a <- b + 2*const ]
	Pattern* Tile1_Addn::try_to_cover(Tree *i, std::vector<Tile*>& all) {
		if (i->op == addn && i->leaves.back()->root->getval() == 1) {
			auto j = i->leaves.front();
			if (j->op == s_ln && j->leaves.back()->root->getval() == 1) {
				auto k = j->leaves.front();
				if (k->op == addn) {
					int64_t n = k->leaves.back()->root->getval();
					auto l = k->leaves.front();
					if (l->op == s_rn && l->leaves.back()->root->getval() == 1) {
						auto m = l->leaves.front();
						auto item = new Num(2 * n);
						auto num = new Tree(item, Op::leaf);
						i->leaves.back() = num;
						i->leaves.front() = m;
					}
				}
			}
		}
		LeavesRecursion(i, all, this);
		return NULL;
	}


	std::string Tile1_Add::printer() { return ""; }                       // LA::[ a <- b + c ] --> L3::[ a <- b + c ][ a-- ]
	Pattern* Tile1_Add::try_to_cover(Tree *i, std::vector<Tile*>& all) {
		if (i->op == addn && i->leaves.back()->root->getval() == 1) {
			auto j = i->leaves.front();
			if (j->op == s_ln && j->leaves.back()->root->getval() == 1) {
				auto k = j->leaves.front();
				if (k->op == add) {
					auto m = k->leaves.front();
					auto n = k->leaves.back();
					if (m->op == s_rn && m->leaves.back()->root->getval() == 1 && n->op == s_rn && n->leaves.back()->root->getval() == 1) {
						auto p = m->leaves.front();
						auto q = n->leaves.front();
						if (p->op != subn && q->op != subn) {
							i->op = subn;
							auto r = i->leaves.front();
							r->op = add;
							r->leaves.front() = p;
							r->leaves.back() = q;
						}
					}
				}
			}
		}
		LeavesRecursion(i, all, this);
		return NULL;
	}


	Tile2_Lea::Tile2_Lea(Item* w1, Item* w2, Item* w3, int64_t E)  // lea : [ w1 @ w2 w3 E ]
	  : w1 {w1}, w2 {w2}, w3 {w3}, E {E} {}
	std::string Tile2_Lea::printer() { 
	  return ItemPrinter(w1) + "@" + ItemPrinter(w2) + ItemPrinter(w3) + std::to_string(E) + "\n";
    }
	Pattern* Tile2_Lea::try_to_cover(Tree *i, std::vector<Tile*>& all) { 
      if(i->op == Op::add) {
		  auto l = i->leaves.front();
		  auto r = i->leaves.back();

		  Pattern* p;
		  auto recursivelyCoverSubTrees = [&](Tree* i1) {
			  if (i1->op != Op::leaf) {
					  p->leaves.push_back(Cover(i1, all));
			  }
		  };

		  if (r->op == Op::s_ln) { auto temp = r; r = l; l = temp; }
		  if (l->op == Op::s_ln) { 
		      int64_t n = l->leaves.back()->root->getval();
			  if (n == 1 || n == 2 || n == 3) {
				  int64_t E = n == 1 ? 2 : n == 2 ? 4 : 8;
				  auto t = new Tile2_Lea(i->root, r->root, l->leaves.front()->root, E);
		          p = new Pattern(t);
			      recursivelyCoverSubTrees(l->leaves.front());
			      recursivelyCoverSubTrees(r);
			      return p;
			  }
		  }
      }
      return NULL;
    }


	Tile2_Cjump::Tile2_Cjump(Item* t1, Op cmp, Item* t2, Item* label)  // cjump : [ cjump t1 cmp t2 label ]
      : t1 {t1}, t2 {t2}, cmp {cmp}, label {label} {}
	std::string Tile2_Cjump::printer() {
	  return " cjump" + ItemPrinter(t1) + OpPrinter(cmp) + ItemPrinter(t2) + ItemPrinter(label) + "\n";
    }
	Pattern* Tile2_Cjump::try_to_cover(Tree *i, std::vector<Tile*>& all) { 
      if(i->op == Op::cjmp) {
		  Tile2_Cjump* t;
		  Pattern* p;
		  auto recursivelyCoverSubTrees = [&](Tree* T) {
			  if (T->op != Op::leaf) {
					  p->leaves.push_back(Cover(T, all));
			  }
		  };

		  auto cond = i->leaves.front();
		  if (cond->op <= c_e && cond->op >= c_l) {
			  auto t1 = cond->leaves.front();
			  auto t2 = cond->leaves.back();
			  if (t1->op == s_rn && t2->op == s_rn && t1->leaves.back()->root->getval() == t2->leaves.back()->root->getval()) {  // LA cmp
				  t = new Tile2_Cjump(t1->leaves.front()->root, cond->op, t2->leaves.front()->root, i->root);
				  p = new Pattern(t);
				  recursivelyCoverSubTrees(t1->leaves.front());
				  recursivelyCoverSubTrees(t2->leaves.front());
			  } else { // merged cjump
				  t = new Tile2_Cjump(t1->root, cond->op, t2->root, i->root);
				  p = new Pattern(t);
				  recursivelyCoverSubTrees(t1);
				  recursivelyCoverSubTrees(t2);
			  }
		  } else {  // basic
			  auto item = new Num(1);
			  t = new Tile2_Cjump(cond->root, c_e, item, i->root);
			  p = new Pattern(t);
			  recursivelyCoverSubTrees(cond);
		  }
		  return p;
      }
      return NULL;
    }


	Tile2_LoadM::Tile2_LoadM(Item* w, Item* x, int64_t M)  // load : [ w <- mem x M ]
      : w {w}, x {x}, M {M} {}
	std::string Tile2_LoadM::printer() { 
	  return ItemPrinter(w) + "<- mem" + ItemPrinter(x) + std::to_string(M) + "\n";
    }
	Pattern* Tile2_LoadM::try_to_cover(Tree *i, std::vector<Tile*>& all) { 
		if(i->op == Op::load) {
			Tree* x = i->leaves.back();
			int64_t M = 0;

			if (x->op == Op::addn)  {
				auto x2 = x->leaves.front();
				int64_t M2 = x->leaves.back()->root->getval();
				if (M2 % 8 == 0) {
					x = x2;
					M = M2;
				}
			}

			auto t = new Tile2_LoadM(i->root, x->root, M);
		    auto p = new Pattern(t);
			SubTreeRecursion(x, p, all);
			return p;
		}
		return NULL;
	}
	

	Tile2_SroreM::Tile2_SroreM(Item* x, int64_t M, Item* s)  // store : [ mem x M <- s ]
      : x {x}, M {M}, s {s} {}
	std::string Tile2_SroreM::printer() { 
	  return " mem" + ItemPrinter(x) + std::to_string(M) + " <-" + ItemPrinter(s) + "\n";
    }
	Pattern* Tile2_SroreM::try_to_cover(Tree *i, std::vector<Tile*>& all) { 
		if(i->op == Op::store) {
			Tree* x = i->leaves.front();
			Tree* s = i->leaves.back();
			int64_t M = 0;

			if (x->op == Op::addn)  {
				auto x2 = x->leaves.front();
				int64_t M2 = x->leaves.back()->root->getval();
				if (M2 % 8 == 0) {
					x = x2;
					M = M2;
				}
			}

			auto t = new Tile2_SroreM(x->root, M, s->root);
		    auto p = new Pattern(t);
			SubTreeRecursion(x, p, all);
			SubTreeRecursion(s, p, all);
			return p;
		}
		return NULL;
	}


    Tile3_PP::Tile3_PP(Item* w, Op op)  // self inc/dec : [ v(++|--) ]
	  : w {w}, op {op} {}
	std::string Tile3_PP::printer() {
	  std::string s = op == Op::addn ? "++" : "--";
	  return " %v" + std::to_string(w->getval()) + s + "\n";
    }
	Pattern* Tile3_PP::try_to_cover(Tree *i, std::vector<Tile*>& all) { 
      if(i->op == Op::addn || i->op == Op::subn) {
		  int64_t r = i->root->getval();
		  int64_t r1 = i->leaves.front()->root->getval();
		  if(r == r1 && i->leaves.back()->root->getval() == 1) {
			  auto t = new Tile3_PP(i->root, i->op);
              auto p = new Pattern(t);
              SubTreeRecursion(i->leaves.front(), p, all);
              return p;
		  }
      }
      return NULL;
    }


	Tile3_SelfOp::Tile3_SelfOp(Item* w, Op op, Item* t) // self aop/sop : [ v += t ]
	  : w {w}, op {op}, t {t} {}
	std::string Tile3_SelfOp::printer() {
	  return ItemPrinter(w) + OpPrinter(op) + ItemPrinter(t) + "\n";
    }
	Pattern* Tile3_SelfOp::try_to_cover(Tree *i, std::vector<Tile*>& all) { 
      if(i->op <= Op::s_rn) {
		  int64_t r = i->root->getval();
		  int64_t r1 = i->leaves.front()->root->getval();
		  if(i->root->type == i->leaves.front()->root->type && r == r1) {   // v = 1 - v
			  auto t = new Tile3_SelfOp(i->root, i->op, i->leaves.back()->root);
              auto p = new Pattern(t);
			  SubTreeRecursion(i->leaves.front(), p, all);
			  SubTreeRecursion(i->leaves.back(), p, all);
              return p;
		  }
      }
      return NULL;
    }

    
	Tile4_AopSop::Tile4_AopSop(Item* w, Item* t1, Item* t2, Op op)  // aop/sop : [ a <- b ][ a += c ]
	  : w {w}, t1 {t1}, t2 {t2}, op {op} {}
	std::string Tile4_AopSop::printer() {
	  std::string s;
	  s += ItemPrinter(w) + "<-" + ItemPrinter(t1) + "\n";
	  if (t2->type == NUM && t2->getval() == 1 && op == addn) {
		  s += " %v" + std::to_string(w->getval()) + "++\n";
	  } else if (t2->type == NUM && t2->getval() == 1 && op == subn) {
		  s += " %v" + std::to_string(w->getval()) + "--\n";
	  } else {
		  s += ItemPrinter(w) + OpPrinter(op) + ItemPrinter(t2) + "\n";
	  }
	  return s;
    }
	Pattern* Tile4_AopSop::try_to_cover(Tree *i, std::vector<Tile*>& all) { 
      if(i->op <= Op::s_rn) {
		  auto t = new Tile4_AopSop(i->root, i->leaves.front()->root, i->leaves.back()->root, i->op);
		  auto p = new Pattern(t);
		  SubTreeRecursion(i->leaves.front(), p, all);
		  SubTreeRecursion(i->leaves.back(), p, all);
		  return p;
      }
      return NULL;
    }

    
    Tile4_Asmt::Tile4_Asmt(Item* w, Item* s)   // assignment : [ v <- s ]
	  : w {w}, s {s} {}
	std::string Tile4_Asmt::printer() {
	  return ItemPrinter(w) + "<-" + ItemPrinter(s) + "\n";
    }
	Pattern* Tile4_Asmt::try_to_cover(Tree *i, std::vector<Tile*>& all) { 
      if(i->op == Op::asmt) {
		  auto t = new Tile4_Asmt(i->root, i->leaves.front()->root);
          auto p = new Pattern(t);
          SubTreeRecursion(i->leaves.front(), p, all);
          return p;  
      }
      return NULL;
    }

    
	Tile4_Cmp::Tile4_Cmp(Item* w, Item* t1, Item* t2, Op cmp)  // cmp : [ w <- t1 cmp t2 ]
	  : w {w}, t1 {t1}, t2 {t2}, cmp {cmp} {}
	std::string Tile4_Cmp::printer() {
	  return ItemPrinter(w) + "<-" + ItemPrinter(t1) + OpPrinter(cmp) + ItemPrinter(t2) + "\n";
    }
	Pattern* Tile4_Cmp::try_to_cover(Tree *i, std::vector<Tile*>& all) { 
      if(i->op <= Op::c_e) {
		  auto t = new Tile4_Cmp(i->root, i->leaves.front()->root, i->leaves.back()->root, i->op);
		  auto p = new Pattern(t);
		  SubTreeRecursion(i->leaves.front(), p, all);
		  SubTreeRecursion(i->leaves.back(), p, all);
		  return p;
      }
      return NULL;
    }


	Tile4_Ret::Tile4_Ret(Item* t)  // return
	  : t {t} {}
	std::string Tile4_Ret::printer() {
	  std::string s;
	  if(t) { s += " rax <-" + ItemPrinter(t) + "\n"; }
	  s += " return\n";
	  return s;
    }
	Pattern* Tile4_Ret::try_to_cover(Tree *i, std::vector<Tile*>& all) { 
      Pattern* p;
      if(i->op == Op::ret) {
		  Tile4_Ret* t;
		  if (i->leaves.size()) {
			  t = new Tile4_Ret(i->leaves.front()->root);
			  p = new Pattern(t);
			  SubTreeRecursion(i->leaves.front(), p, all);
		  } else {
			  t = new Tile4_Ret(NULL);
			  p = new Pattern(t);
		  }
		  return p;
      }
      return NULL;
    }

    
	Tile4_Label::Tile4_Label(Item* label, Op op) // goto/label
	  : label {label}, op {op} {}
    std::string Tile4_Label::printer() {
	  std::string s;
	  if(op == Op::br) { s += " goto"; }
	  s += ItemPrinter(label) + "\n";
	  return s;
    }
	Pattern* Tile4_Label::try_to_cover(Tree *i, std::vector<Tile*>& all) {
      if(i->op == Op::br || i->op == Op::label) {
		  auto t = new Tile4_Label(i->root, i->op);
		  auto p = new Pattern(t);
		  return p;
      }
      return NULL;
    }

    
	Tile4_Call::Tile4_Call(Tree *t) // call
	  : t {t} {}
	Pattern* Tile4_Call::try_to_cover(Tree *i, std::vector<Tile*>& all) { 
      if(i->op == Op::call) {
		  auto t = new Tile4_Call(i);
		  auto p = new Pattern(t);
		  return p;
      }
      return NULL;
    }
    std::string Tile4_Call::printer() {
	  std::string s;
	  std::string callee;
	  bool ifRt = false;
	  std::vector<std::string> regs ({" rdi <-", " rsi <-", " rdx <-", " rcx <-", " r8 <-", " r9 <-"});
	  int n = t->leaves.size() - 2;
	  int regarg = n > 6 ? 6 : n;
	  int stackarg = n > 6 ? (n-6) : 0;
	  
	  callee = ItemPrinter(t->leaves.back()->root);
	  if(callee[1] != '@' && callee[1] != '%') {
		  ifRt = true;
	  }
	  t->leaves.pop_back();
	  
	  if(!ifRt) {
		  s += " mem rsp -8 <-" + ItemPrinter(t->leaves.back()->root) + "\n";
	  }
	  for(int i = 0; i < regarg; ++i) {
		  s += regs[i] + ItemPrinter((t->leaves)[i]->root) + "\n";
	  }
	  for(int i = 0; i < stackarg; ++i) {
		  s += " mem rsp " + std::to_string(-16 - 8 * i) + " <-" + ItemPrinter((t->leaves)[6 + i]->root) + "\n";
	  }
	  
	  s += " call" + callee + std::to_string(n) + "\n";
	  
	  if(!ifRt) {
		  s += ItemPrinter(t->leaves.back()->root) + "\n";
	  }
	  
	  if(t->root != NULL) {
		  s += ItemPrinter(t->root) + "<- rax" + "\n";
	  }
	  return s;
    }

}