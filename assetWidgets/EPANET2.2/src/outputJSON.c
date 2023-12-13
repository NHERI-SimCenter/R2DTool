//
// written: fmk 10/23
//

#include <stdlib.h>
#include <stdio.h>
#include "types.h"
//#include "enumstxt.h"

static inline const char *getStatusType(int f)
{
  static const char *strings[] = { "XHEAD", "TEMPCLOSED", "CLOSED", "OPEN", "ACTIVE", "XFLOW", "XFCV", "XPRESSURE", "FILLING", "EMPTYING", "OVERFLOWING"};
  return strings[f];
}  
  
static inline const char *getMixType(int f)
{
  static const char *strings[] = { "MIX1", "MIX2", "FIFO", "LIFO"};
  return strings[f];
}

static inline const char *getPumpType(int f)
{
  static const char *strings[] = { "CUSTOM_HP", "POWER_FUNC", "CUSTOM", "NOCURVE"};
  return strings[f];
}

int outputJSON(Project *theProjectPtr, const char *fileName) {

  //
  // open file, error message and return -1 if error
  //
  
  FILE* jsonFile = fopen(fileName, "w"); 
  if (jsonFile == 0) {
    printf("ERROR: Coul not open file: %s\n",fileName);
    return -1;
  }

  //
  // start of GeoJSON
  //
  
  fprintf(jsonFile, "\n{\n \"type\":\"FeatureCollection\", \"features\":[");

  int numNodes = theProjectPtr->network.Nnodes;
  int numTanks = theProjectPtr->network.Ntanks;
  int numJuncs = theProjectPtr->network.Njuncs;
  int numLinks = theProjectPtr->network.Nlinks;
  int numPipes = theProjectPtr->network.Npipes;
  int numPumps = theProjectPtr->network.Npumps; 
  int numValves = theProjectPtr->network.Nvalves;


  printf("\n\t numNodes %d, numTanks %d, numJuncs %d, numLinks %d, numPipes %d, numPumps %d numValves: %d.\n",
	 numNodes,
	 numTanks,
	 numJuncs,
	 numLinks,
	 numPipes,
	 numPumps,
	 numValves);  
  
  //
  // output Junctions
  //
  
  for (int i=1; i<=numNodes; i++) { // not standard C indexing
    Snode *theNode = &theProjectPtr->network.Node[i];
    if (theNode->Type == JUNCTION) {
      if (i == 1) // the == 1 is needed to for no initial comma
	fprintf(jsonFile,"\n {\"type\":\"Feature\",\"geometry\":{ \"type\":\"Point\", \"coordinates\":[%f,%f]},\"id\":1, \"properties\":{\"type\":\"Junction\",\"id\":\"%s\", \"El\":%f, \"C0\":%f, \"Ke\":%f}}",
		theNode->X,
		theNode->Y,
		theNode->ID,
		theNode->El,	       
		theNode->C0,
		theNode->Ke);
      else
	fprintf(jsonFile,",\n {\"type\":\"Feature\",\"geometry\":{ \"type\":\"Point\", \"coordinates\":[%f,%f]}, \"id\":%d, \"properties\":{\"type\":\"Junction\",\"id\":\"%s\", \"El\":%f, \"C0\":%f, \"Ke\":%f}}",
		theNode->X,
		theNode->Y,
		i,
		theNode->ID,
		theNode->El,	       
		theNode->C0,
		theNode->Ke);	    
    }
  }

  //
  // output Pumps
  //

  for (int i=1; i<=numPumps; i++) { // not standard C indexing
    Spump *thePump = &theProjectPtr->network.Pump[i];
    Slink *theLink = &theProjectPtr->network.Link[thePump->Link];
    Snode *startNode= &theProjectPtr->network.Node[theLink->N1];
    Snode *endNode = &theProjectPtr->network.Node[theLink->N2];
    fprintf(jsonFile,",\n {\"type\":\"Feature\",\"geometry\":{ \"type\":\"LineString\", \"coordinates\":[[%f,%f],[%f,%f]]},\"id\":%d, \"properties\":{\"type\":\"Pump\",\"id\":\"%s\", \"Ptype\":\"%s\", \"Q0\":%f, \"Qmax\":%f, \"Hmax\": %f, \"H0\":%f, \"R\":%f, \"N\":%f, \"Hcurve\":%d, \"Ecurve\":%d, \"Upat\":%d,\"Epat\":%d, \"Ecost\":%f,\"startNode\":\"%s\", \"endNode\":\"%s\"}}",
	    startNode->X,
	    startNode->Y,
	    endNode->X,
	    endNode->Y,
	    i,
	    theLink->ID,
	    getPumpType(thePump->Ptype),
	    thePump->Q0,
	    thePump->Qmax,
	    thePump->Hmax,
	    thePump->H0,
	    thePump->R,
	    thePump->N,
	    thePump->Hcurve,
	    thePump->Ecurve,
	    thePump->Upat,
	    thePump->Epat,	     
	    thePump->Ecost,
	    startNode->ID,
	    endNode->ID);	   

  }

  //
  // output Pipes
  //
  
  for (int i=1; i<=numLinks; i++) { // not standard C indexing
    Slink *theLink = &theProjectPtr->network.Link[i];
    int type = theLink->Type;
    char *ID = theLink->ID;
    Snode *startNode = &theProjectPtr->network.Node[theLink->N1];
    Snode *endNode = &theProjectPtr->network.Node[theLink->N2];
    if (theLink->Type == PIPE) {
      fprintf(jsonFile,",\n {\"type\":\"Feature\",\"geometry\":{ \"type\":\"LineString\", \"coordinates\":[[%f,%f],[%f,%f]]},\"id\": %d, \"properties\":{\"type\":\"Pipe\",\"id\":\"%s\", \"Diam\":%f, \"Kc\":%f, \"Len\":%f, \"Status\":\"%s\", \"Km\":%f, \"Kb\":%f, \"Kw\":%f, \"R\":%f,\"Rc\":%f,\"startNode\":\"%s\", \"endNode\":\"%s\"}}",
	      startNode->X,
	      startNode->Y,
	      endNode->X,
	      endNode->Y,
	      i,
	      theLink->ID,
	      theLink->Diam,
	      theLink->Kc,
	      theLink->Len,
	      getStatusType(theLink->Status),
	      theLink->Km,
	      theLink->Kb,
	      theLink->Kw,
	      theLink->R,
	      theLink->Rc,
	      startNode->ID,
	      endNode->ID);
      
      
    } else if (theLink->Type == CVPIPE) {
      
    } else if (theLink->Type == PUMP) {
      ; // pumps already output
    } else {  //some type valve
      
    }
  }

  //
  // output Reservoirs and Tanks
  //

  for (int i=1; i<=numTanks; i++) { // not standard C indexing
    Stank *theTank = &theProjectPtr->network.Tank[i];      
    int nodeID = theTank->Node;
    double A = theTank->A;
    Snode *theNode = &theProjectPtr->network.Node[nodeID];      
    if (A == 0) {   // keeping tanks and resrvoirs seperate for now .. might not want all the stuff in the structure depending on type
      fprintf(jsonFile,",\n {\"type\":\"Feature\",\"geometry\":{ \"type\":\"Point\", \"coordinates\":[%f,%f]},\"id\":%d, \"properties\":{\"type\":\"Reservoir\",\"id\":\"%s\", \"H0\":%f, \"Vmin\":%f, \"Vmax\":%f, \"V0\": %f, \"Kb\":%f, \"V\":%f, \"C\":%f, \"Pat\":%d, \"Vcurve\":%d, \"MixModel\":\"%s\", \"V1Max\":%f, \"CanOverflow\":%d}}",
	      theNode->X,
	      theNode->Y,
	      i,
	      theNode->ID,
	      theTank->H0,
	      theTank->Vmin,
	      theTank->Vmax,
	      theTank->V0,
	      theTank->Kb,
	      theTank->V,
	      theTank->C,
	      theTank->Pat,
	      theTank->Vcurve,
	      getMixType(theTank->MixModel),	       
	      theTank->V1max,
	      theTank->CanOverflow);		    
    } else {
      fprintf(jsonFile,",\n {\"type\":\"Feature\",\"geometry\":{ \"type\":\"Point\", \"coordinates\":[%f,%f]}, \"id\":%d, \"properties\":{\"type\":\"Tank\",\"id\":\"%s\", \"H0\":%f, \"Vmin\":%f, \"Vmax\":%f, \"V0\": %f, \"Kb\":%f, \"V\":%f, \"C\":%f, \"Pat\":%d, \"Vcurve\":%d, \"MixModel\":\"%s\", \"V1Max\":%f, \"CanOverflow\":%d}}",
	      theNode->X,
	      theNode->Y,
	      i,
	      theNode->ID,
	      theTank->H0,
	      theTank->Vmin,
	      theTank->Vmax,
	      theTank->V0,
	      theTank->Kb,
	      theTank->V,
	      theTank->C,
	      theTank->Pat,
	      theTank->Vcurve,
	      getMixType(theTank->MixModel),	       
	      theTank->V1max,
	      theTank->CanOverflow);		          
    }
  }

  // finish the JSON file
  fprintf(jsonFile, "\n]}\n");

  fclose(jsonFile);

  return 0;
}
