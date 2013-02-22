// $Id:$
//====================================================================
//  AIDA Detector description implementation for LCD
//--------------------------------------------------------------------
//
//  Generic ROOT based geometry display program
// 
//  Author     : M.Frank
//
//====================================================================
#include "run_plugin.h"

//______________________________________________________________________________
namespace {
  void usage() {
    cout << "geoPlugin -opt [-opt]                                                   \n"
      "        -plugin <name>  [REQUIRED]  Plugin to be executed and applied.        \n"
      "        -input  <file>  [REQUIRED]  Specify input file.                       \n"
	 << endl;
    exit(EINVAL);
  }
}

//______________________________________________________________________________
int main(int argc,char** argv)  {
  string plugin;
  vector<char*> geo_files;
  for(int i=1; i<argc;++i) {
    if ( argv[i][0]=='-' ) {
      if ( strncmp(argv[i],"-input",2)==0 )
	geo_files.push_back(argv[++i]);
      else if ( strncmp(argv[i],"-plugin",2)==0 )
        plugin = argv[++i];
      else
	usage();
    }
    else {
      usage();
    }
  }
  if ( geo_files.empty() || plugin.empty() )
    usage();

  LCDD& lcdd = LCDD::getInstance();  
  // Load compact files
  run_plugin(lcdd,"DD4hepCompactLoader",int(geo_files.size()),&geo_files[0]);
  // Execute plugin
  run_plugin(lcdd,plugin.c_str(),0,0);
  return 0;
}