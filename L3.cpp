#include "L3.h"

namespace L3 {

Item::Item (ItemType n)
  : type {n}{}

int64_t Item::getval() {
    return 0;
}

Var::Var (int n)
  : Item(ItemType::VAR) {
      var = n;
  return ;
}

int64_t Var::getval() {
    return var;
}

Num::Num (int64_t n)
  : Item(ItemType::NUM){
      num = n;
  return ;
}

int64_t Num::getval() {
    return num;
}

Label::Label (int n)
   : Item(ItemType::LABEL){
  label = n;
  return ;
}

int64_t Label::getval() {
    return label;
}

FunName::FunName (std::string s)
    : Item(ItemType::FUN){
  fun = s;
  return ;
}

Tree::Tree (Item* i, Op o)
  : root {i},
    op {o} {
        id_in_func = 0;
        return;
    }

Pattern::Pattern (Tile* t)
  : tile {t} {
        return;
    }


}