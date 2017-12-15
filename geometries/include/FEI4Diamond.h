#ifndef FEI4Diamond_h
#define FEI4Diamond_h

/** @class FEI4Diamond
      * This class is the implementation of  @class EUTelGenericPixGeoDescr
      * for a diamond with rectangular and hexagonal cells bonded to a FE-I4
  */

// EUTELESCOPE
#include "EUTelGenericPixGeoDescr.h"

// ROOT
#include "TGeoMaterial.h"
#include "TGeoMedium.h"
#include "TGeoVolume.h"
#include "TGeoArb8.h"

namespace eutelescope {
	namespace geo {

		class FEI4Diamond : public EUTelGenericPixGeoDescr {

		public:
			FEI4Diamond();
			~FEI4Diamond();

			void createRootDescr(char const *);
			std::string getPixName(int, int);
			std::pair<int, int> getPixIndex(char const *);

		protected:
			TGeoMaterial *matDia;
			TGeoMedium *Dia;
			TGeoVolume *plane;
			
			// Layout pixel values starting at (0,0)
			int layoutStartX = 61;
			int layoutStartY = 0;
			int layoutEndX = 78;
			int layoutEndY = 92;
			int metalStartX = 64;
			int metalEndX = 76;
			int metalStartY = 13;
			int metalEndY = 76;
			int hexStartY = 43;
			bool evendCol = false;
			
		};

		extern "C" {
		EUTelGenericPixGeoDescr *maker();
		}

	} // namespace geo
} // namespace eutelescope

#endif // FEI4DIAMOND_H
