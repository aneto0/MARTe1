/*
 * Copyright 2011 EFDA | European Fusion Development Agreement
 *
 * Licensed under the EUPL, Version 1.1 or - as soon they 
   will be approved by the European Commission - subsequent  
   versions of the EUPL (the "Licence"); 
 * You may not use this work except in compliance with the 
   Licence. 
 * You may obtain a copy of the Licence at: 
 *  
 * http://ec.europa.eu/idabc/eupl
 *
 * Unless required by applicable law or agreed to in 
   writing, software distributed under the Licence is 
   distributed on an "AS IS" basis, 
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either 
   express or implied. 
 * See the Licence for the specific language governing 
   permissions and limitations under the Licence. 
 *
 * $Id$
 *
**/
/**
 * @file shows how to use LinkedListables and filters
 */
#include "ErrorManagement.h"
#include "Iterators.h"
#include "LinkedListable.h"

/**
 * LinkedListable of Humans. Each human is described
 * by an height and a name
 */
class Human : public LinkedListable{
private:
    float height;
    char *name;

public:
    Human(const char *n, float h){
        height = h;
        if(n != NULL){
            name = (char *)malloc(strlen(n) + 1);
            strcpy(name, n);
        }
        else{
            name = NULL;
        }
    }

    virtual ~Human(){
        if(name != NULL){
            free((void *&)name);
        }
    }
    
    const char *Name(){
        return name;
    }
    
    float Height(){
        return height;
    }
};

/*
 * An iterator which prints all the names and heights
 */
class HumanIterator : public Iterator{
    
    virtual void Do(LinkedListable *data){
        Human *h = dynamic_cast<Human *>(data);
        if(h == NULL){
            return;
        }
        CStaticAssertErrorCondition(Information, "My name is %s and my height is %f", h->Name(), h->Height());
    }
};

/*
 * A sorter for the human names
 */
class HumanSorter : public SortFilter{
    virtual int32 Compare(LinkedListable *data1,LinkedListable *data2){
        Human *h1 = dynamic_cast<Human *>(data1);
        Human *h2 = dynamic_cast<Human *>(data2);
        if((h1 == NULL) || (h2 == NULL)){
            return 0;
        }
        return strcmp(h1->Name(), h2->Name()); 
    }
};

/**
 * Search filter for the human first character. Will return True if and only if
 * the first character in the name is as requested in the constructor
 */
class HumanSearchFilter : public SearchFilter{
private:
    char firstChar;
public:
    HumanSearchFilter(char fc){
        firstChar = fc;
    }

    /** the function that performs the search on a set of data */
    bool Test(LinkedListable *data){
        Human *h = dynamic_cast<Human *>(data);
        if(h == NULL){
            return False;
        }
        if(h->Name() == NULL){
            return False;
        }
        return h->Name()[0] == firstChar;
    }
};




int main(int argc, char *argv[]){
    //Output logging messages to the console
    LSSetUserAssembleErrorMessageFunction(NULL); 

    //Create a root node
    LinkedListable root;
    //Create some humans and link them
    Human tim("Tim", 1.75);
    root.Insert(&tim);
    Human tom("Tom", 1.85);
    tim.Insert(&tom);
    Human john("John", 1.66);
    tom.Insert(&john);
    Human sue("Sue", 1.78);
    john.Insert(&sue);
    Human sam("Sam", 1.88);
    sue.Insert(&sam);

    //Cycle through all the humans and print their name and height
    LinkedListable *list = &root;
    int32 i=0;
    while(list != NULL){
        Human *h = dynamic_cast<Human *>(list);
        if(h != NULL){
            CStaticAssertErrorCondition(Information, "[%d]:My name is %s and my height is %f", i, h->Name(), h->Height());
        }
        list = list->Next();
        i++;
    }

    //Repeat the same exercise but using an iterator
    //reset the list to the starting point
    list = &root;
    //Create the iterator and iterate
    HumanIterator   hi;
    CStaticAssertErrorCondition(Information, "Printing from iterator");
    list->Iterate(&hi);

    //Sort by name
    //reset the list to the starting point
    list = &root;
    //Create the sorter
    HumanSorter sorter;
    list->BSort(&sorter);

    //Print again...
    CStaticAssertErrorCondition(Information, "Printing from iterator, after being sorted");
    list->Iterate(&hi);

    //Create a search filter for all the names starting with a T
    HumanSearchFilter humanSearchT('T');
    CStaticAssertErrorCondition(Information, "Printing only names starting with a T");
    //reset the list to the starting point
    list                  = &root;
    LinkedListable *found =  list;
    while(found != NULL && list != NULL){
        found = list->Search(&humanSearchT);
        if(found != NULL){
            Human *h = dynamic_cast<Human *>(found);
            CStaticAssertErrorCondition(Information, "My name is %s and my height is %f", h->Name(), h->Height());
            list     = found->Next();
        }
    } 

    //Create a search filter for all the names starting with a S
    HumanSearchFilter humanSearchS('S');
    CStaticAssertErrorCondition(Information, "Printing only names starting with a S");
    //reset the list to the starting point
    list  = &root;
    found =  list;
    while(found != NULL && list != NULL){
        found = list->Search(&humanSearchS);
        if(found != NULL){
            Human *h = dynamic_cast<Human *>(found);
            CStaticAssertErrorCondition(Information, "My name is %s and my height is %f", h->Name(), h->Height());
            list     = found->Next();
        }
    }

    //Extract all elements starting with a T
    list     = &root;
    while(list->Extract(&humanSearchT) != NULL);
    CStaticAssertErrorCondition(Information, "Printing full list after removing names starting with a T");
    //Iterate again
    list     = &root;
    list->Iterate(&hi);
    return 0;
}

