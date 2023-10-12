#include <string>
#include <iostream>
#include <fstream>
#include <cassert>



#include <merge.h>

//#define NDEBUG

using namespace std;

namespace L3{
  bool sets_cmp_insert(std::set<int>* b, std::set<int>* a);
  bool sets_cmp_erase(std::set<int>* b, std::set<int>* a);
  bool sets_cmp_intersect(std::set<int>* b, std::set<int>* a);
  void graft(Tree* a, Tree* b);

  /* liveness analysis */
  void MergeTree(Program &p) {
    for(auto f : p.functions) {
        int size = f->tree_count;
        std::vector<std::set<int>*> GEN (size);
        std::vector<std::set<int>*> KILL (size);
        std::vector<std::set<int>*> IN (size);
        std::vector<std::set<int>*> OUT (size);

        std::vector<bool> JUMP (size, false);
        std::vector<int> SUCCESSOR (size, 0); //no log for adjacent successor

        /* generate the GEN and KILL set */
        for(auto c : f->contexts) {
	        for(auto t : c->trees) {
                int i = t->id_in_func;
		        GEN[i] = new std::set<int>;
                KILL[i] = new std::set<int>;
                IN[i] = new std::set<int>;
                OUT[i] = new std::set<int>;
            
                if(t->op < Op::cjmp) {
                    if(t->root != NULL && t->root->type == VAR) {
                        KILL[i]->insert(t->root->getval());
                    }
                    for(auto l : t->leaves) {
                        if(l->root->type == VAR) {
                            GEN[i]->insert(l->root->getval());
                        }
                    }
                    if (t->op == Op::ret) {
                        SUCCESSOR[i] = 1;
                    }
                } else if (t->op == Op::call) {
                    if(t->root != NULL) {
                        KILL[i]->insert(t->root->getval());
                    }
                    for(int it = 0; it < t->leaves.size() - 2; it++) {
                        if((t->leaves)[it]->root->type == ItemType::VAR) {
                            GEN[i]->insert((t->leaves)[it]->root->getval());
                        }
                    }
                    if(t->leaves.back()->root->type == ItemType::VAR) {
                        GEN[i]->insert((t->leaves).back()->root->getval());
                    } else if ((dynamic_cast<FunName*>(t->leaves.back()->root)->fun)[0] == 't') {
                        SUCCESSOR[i] = 1;
                    }
                } else if (t->root->type == LABEL) {  //with dst of label
                    if(t->op != Op::label) {
                        JUMP[i] = true;
                        if(t->op == Op::br) {  //goto
                            SUCCESSOR[i] = f->label_id_map.find(t->root->getval())->second;
                        } else {  //cjmp
                            SUCCESSOR[i] = -1 - f->label_id_map.find(t->root->getval())->second;
                            GEN[i]->insert((t->leaves)[0]->root->getval());
                        }
                    }
                } else {
                    assert(0);
                }
	        }
        }

        /* generate the IN and OUT set */
        bool isModified = false;
        bool isInitial = true;
        do {
            isModified = false;
            for(int i = size - 1; i >= 0; --i) {
                if(JUMP[i] == false) {
                    if(SUCCESSOR[i] == 0) { //normal
                        isModified = sets_cmp_insert(OUT[i], IN[i + 1]) || isModified;
                        isModified = sets_cmp_intersect(OUT[i], IN[i + 1]) || isModified;
                    }
                } else {
                    if(SUCCESSOR[i] < 0) { //cjump
                        std::set<int>* UNITE = new std::set<int>;
                        sets_cmp_insert(UNITE, IN[i + 1]);
                        sets_cmp_insert(UNITE, IN[-1 - SUCCESSOR[i]]);
                        isModified = sets_cmp_insert(OUT[i], UNITE) || isModified;
                        isModified = sets_cmp_intersect(OUT[i], UNITE) || isModified;
                        delete UNITE;
                    } else { //goto
                        isModified = sets_cmp_insert(OUT[i], IN[SUCCESSOR[i]]) || isModified;
                        isModified = sets_cmp_intersect(OUT[i], IN[SUCCESSOR[i]]) || isModified;
                    }
                }
                if(isModified || isInitial) {  // deal with the IN set only when necessary
                    std::set<int>* UNITE = new std::set<int>;
                    sets_cmp_insert(UNITE, OUT[i]);
                    sets_cmp_erase(UNITE, KILL[i]);
                    sets_cmp_insert(UNITE, GEN[i]);
                    sets_cmp_insert(IN[i], UNITE);
                    sets_cmp_intersect(IN[i], UNITE);
                    delete UNITE;
                }
            }
            isInitial = false;
        } while(isModified);
    
        for(auto c : f->contexts) { 
            int n = c->trees.size();
            for(int i = 0; i < n - 1; ++i) {
                int treeI = (c->trees)[i]->id_in_func;
                if(KILL[treeI]->empty()) {
                    continue;
                }
                auto rootI = *KILL[treeI]->begin();
                if(OUT[treeI]->find(rootI) == OUT[treeI]->end()) {
                    c->trees.erase(c->trees.begin() + i);
                    i--;
                    n--;
                }
            }
        }

        /* merge */
        for(auto c : f->contexts) {
            int n = c->trees.size();
            for(int i = 0; i < n - 1; ++i) {
                for(int j = i + 1; j < n; ++j) {

                    int treeI = (c->trees)[i]->id_in_func;
                    int treeJ = (c->trees)[j]->id_in_func;
                    auto leavesJ = (c->trees)[j]->leaves;
                    
                    if(KILL[treeI]->empty()) {
                        break;
                    }
                    auto rootI = *KILL[treeI]->begin();

                    if(GEN[treeJ]->find(rootI) != GEN[treeJ]->end()) {   // rootI-leafJ match, to merge or to break treeI

                        if(OUT[treeJ]->find(rootI) != OUT[treeJ]->end() && KILL[treeJ]->find(rootI) == KILL[treeJ]->end()  // rootI living, cannot merge
                           || leavesJ.size() == 2 && leavesJ.front()->root->getval() == leavesJ.back()->root->getval()) {  // dupicated leaves of treeJ
                            break;
                        } else {   // merge
                            sets_cmp_insert(GEN[treeJ], GEN[treeI]);   // merge the GEN set
                            graft((c->trees)[i], (c->trees)[j]);
                            c->trees.erase(c->trees.begin() + i);
                            i--;
                            n--;
                            break;
                        }
                    } else {
                        if(!KILL[treeJ]->empty()) {
                            auto rootJ = *KILL[treeJ]->begin();
                            if(rootI == rootJ) {  //redefine of rootI
                                break;
                            }
                            if(GEN[treeI]->find(rootJ) != GEN[treeI]->end()) {  //redefine of leavesI
                                break;
                            }
                    
                        }
                    }
                }
            }
        }
    }

    return ;
  }

    bool sets_cmp_insert(std::set<int>* b, std::set<int>* a) {
        bool isModified = false;
        for (std::set<int>::iterator it = (*a).begin(); it != (*a).end(); ++it) {
            if ((*b).find(*it) == (*b).end()) {
                isModified = true;
                (*b).insert(*it);
            }
        }
        return isModified;
    }

    bool sets_cmp_erase(std::set<int>* b, std::set<int>* a) {
        bool isModified = false;
        for (auto it = (*a).begin(); it != (*a).end(); ++it) {
            if ((*b).find(*it) != (*b).end()) {
                isModified = true;
                (*b).erase(*it);
            }
        }
        return isModified;
    }

    bool sets_cmp_intersect(std::set<int>* b, std::set<int>* a) {
        bool isModified = false;
        for (auto it = (*b).begin(); it != (*b).end(); ++it) {
            if ((*a).find(*it) == (*a).end()) {
                isModified = true;
                (*b).erase(*it);
            }
        }
        return isModified;
    }

    void graft(Tree* a, Tree* b) {
        for(auto& i : b->leaves) {
            if(i->root->getval() == a->root->getval()) {
                i = a;
                return;
            }
        }
    }
}
