#ifndef RUNLUMIEVENTKEY_H
#define RUNLUMIEVENTKEY_H

//unsigned long long gives 19 safe digits for a key
const unsigned long long runMult = 1;
const unsigned long long lumiMult = 1000000;//run < 1000000 always
const unsigned long long evtMult = 10000000000;// lumi < 10000 always; Evt must be  < 10^10
const unsigned long long evtLimit = 10000000000;

unsigned long long keyFromRunLumiEvent(unsigned int run, unsigned int lumi, unsigned long long event)
{
  unsigned long long key = 0;
  if(event >= evtLimit){
    std::cout << "RUNLUMIEVENTKEY WARNING: \'" << event << "\' is greater than event limit 10^10. returning key 0" << std::endl;
    return key;
  }

  key += runMult*((unsigned long long)run);
  key += lumiMult*((unsigned long long)lumi);
  key += evtMult*event;
  return key;
}

/* Use as in following pseudocode:
   std::map<unsigned long long, int> runLumiEvtToEntryMap;
   for(loop over tree1){
     tree1->GetEntry(i);
     unsigned long long key = keyFromRunLumiEvent(run, lumi, evt);
     runLumiEvtToEntryMap[key] = i;
   }

   //OLD BAD VERSION!!!
   for(loop over tree2){
     tree2->GetEntry(i);
     unsigned long long key = keyFromRunLumiEvent(run, lumi, evt);
     tree1->GetEntry(map[key]);

     Do stuff
   }

   //NEW CORRECT VERSION!!!
   for(loop over tree2){
     tree2->GetEntry(i);
     unsigned long long key = keyFromRunLumiEvent(run, lumi, evt);
     int entry = -1;
     if(map.count(key) == 0) continue;
     else entry = map[key];
     tree1->GetEntry(entry);

     Do stuff
   }

   Note this will still be slow because unordered get entry is slower than ordered. 
*/

#endif
