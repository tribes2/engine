//maccarb_common_prefix.h


// for Project Builder, not sure what the heck it sets.
//#ifndef __POWERPC__
//#define __POWERPC__            1
//#endif

                                 
#define TARG_MACCARB             0x0104      // the carbon version # we are targeting.
#define TARG_MACANY              1

// activate custom features.
#define TRUFORM                  1
#define TERRAIN_NORMALS          1   // which will allow Truform on terrain.


// defines for the mac headers to activate proper Carbon codepaths.
#define TARGET_API_MAC_CARBON    1
#define OTCARBONAPPLICATION      1   // means we can use the old-style funcnames
