#include <string>
#include <iostream>
#include <fstream>
#include <vector>

#include <code_generator.h>
#include <tile.h>

using namespace std;

namespace L3{

  /* post order traversal */
  std::string print(Pattern* pt) {
      std::string s;
      while(!pt->leaves.empty()) {
          s += print(pt->leaves.back());
          pt->leaves.pop_back();
      }
      s += pt->tile->printer();
      return s;
  }

  void GenerateCode(Program p){

    /* 
     * Open the output file.
     */ 
    std::ofstream outputFile;
    outputFile.open("prog.L2");
   
    /* 
     * Generate target code
     */ 
    outputFile << "(" << p.entryPointLabel << "\n";
    
    for(auto f : p.functions) {
        outputFile << "(" << f->name << "\n";

        int n = f->args.size();
        outputFile << f->args.size() << "\n";

        std::vector<std::string> regs ({" <- rdi\n", " <- rsi\n", " <- rdx\n", " <- rcx\n", " <- r8\n", " <- r9\n"});
	    int regarg = n > 6 ? 6 : n;
	    int stackarg = n > 6 ? (n-6) : 0;

        for(int i = 0; i < regarg; ++i) {
			outputFile << " %v" << (f->args)[i]->getval() << regs[i];
	    }

	    for(int i = 0; i < stackarg; ++i) {
            outputFile << " %v" << (f->args)[6 + i]->getval() << " <- stack-arg " << 8 * (n - i - 7) << "\n";
	    }

		for(auto c : f->contexts) {
			for(auto pt : c->patterns) {
				outputFile << print(pt);
			}
		}

        outputFile << ")\n";
    }

    outputFile << ")\n";
    /* 
     * Close the output file.
     */ 
    outputFile.close();
   
    return ;
  }
}