#include "FEI4Diamond.h"

namespace eutelescope {
	namespace geo {

		FEI4Diamond::FEI4Diamond()
						: EUTelGenericPixGeoDescr(4.750, 4.650, 0.824, // size X, Y, Z
																			61, 78, 0, 92,      // min max X,Y
																			12.130681818)          // rad length cm
		{
			// TODO X, Y values?
			//--- define material
			matDia = new TGeoMaterial("Dia", 12.0107, 6, 3.52, -_radLength, 45.2); //TODO last parameter absorption length?
			//--- define media
			Dia = new TGeoMedium("Diamond", 1, matDia);
			// Layout values, values in mm, values halfed because of TGeo convention
			double xLayout = 2.375; // 4.750, counting FE-i4 pixel
			double yLayout = 2.325; // 4.650
			double thickness = 0.412; // 500 mu diamond + 250mu Si FE(?), adding the equivalent length of the fraction of the radlength of Si to the diamond thickness -> 500 + 324 = 824
			
			double xLeft = 0.375; //0.750;
			double xMid = 1.5;//3.0;
			double yTop = 0.325;//0.650;
			double yBot = 0.425;//0.85;
			double xRight = 0.375;//.750;
			
			double yRectPart = 0.75;//1.5;
			double rectRows = 15;
			//double rectCols = ;
			double yHexPart = 0.825;//1.65;
			double hexRows = 8; // 7 + one special one
			//double hexCols = ;
			
			// hex cell parts
			double hexSideX = 340.0/3.0/2.0/2.0/1000.0;// /2 for TGeo convention, /1000 mu->cm
			double hexSideY = 200.0/3.0/2.0/1000.0;
			double hexRectBigX =80.0/2.0/1000.0;
			double hexRectBigY =2.0*hexSideY;
			double hexRectSmallX = hexRectBigX;
			double hexRectSmallY = hexSideY;
			
			// rect cell parts
			double rectX = 0.0625;//0.125
			double rectY = 0.05;//0.1
			
			// free cell parts, FE-I4 pixel
			double pixelX = 250.0/2.0/1000.0;
			double pixelY = 50.0/2.0/1000.0;
			
			double leftRows = 93;
			double rightRows = 93;
			double topRows = 13;
			double botRows = 17;
			double midCols = 12;
			
			// used rotations
			TGeoRotation *rotY90 = new TGeoRotation();
			rotY90->RotateY(90);
			TGeoRotation *rotY180 = new TGeoRotation();
			rotY180->RotateY(180);
			TGeoRotation *rotY270 = new TGeoRotation();
			rotY270->RotateY(-90);
			
			//--- make the top container volume
			plane = _tGeoManager->MakeBox("FEI4Dia", Dia, xLayout, yLayout, thickness);
			
			// Volumes without metallisation 
			TGeoVolume *leftFree = _tGeoManager->MakeBox("LeftFree", Dia, xLeft, yLayout, thickness);
			TGeoVolume *topFree = _tGeoManager->MakeBox("TopFree", Dia, xMid, yTop, thickness);
			TGeoVolume *botFree = _tGeoManager->MakeBox("BotFree", Dia, xMid, yBot, thickness);
			TGeoVolume *rightFree = _tGeoManager->MakeBox("RightFree", Dia, xRight, yLayout, thickness);
			
			TGeoVolume *rectPart = _tGeoManager->MakeBox("RectPart", Dia, xMid, yRectPart, thickness);
			TGeoVolume *hexPart = _tGeoManager->MakeBox("HexPart", Dia, xMid, yHexPart, thickness);
			
			//___________________________________________________________________________________________________________
			// rect grid parts
			// double columns even and odd, even has the extra rect cell in the middle
			TGeoVolume *rectdColEven = new TGeoVolumeAssembly("rectdColEven");
			TGeoVolume *rectdColOdd = new TGeoVolumeAssembly("rectdColOdd");
			// split double columns in left and right, right is constructed by rotating left by 180 degrees around y axis
			TGeoVolume *rectdColEvenLeft = new TGeoVolumeAssembly("rectdColEvenLeft");
			TGeoVolume *rectdColOddLeft = new TGeoVolumeAssembly("rectdColOddLeft");
			// define building parts of the columns respectivly, this are left parts, rotate for right parts later
			TGeoVolume *rectdColEvenLeftPart = new TGeoVolumeAssembly("rectdColEvenPart");
			TGeoVolume *rectdColOddLeftPart = new TGeoVolumeAssembly("rectdColOddPart");
			
			// parts of the column part
			// small rectangle
			TGeoVolume *rectSmallRect = _tGeoManager->MakeBox("SmallRect", Dia, rectX/2.0, rectY, thickness);
			// normal rect cell
			TGeoVolume *rectRect = _tGeoManager->MakeBox("RectRect", Dia, rectX, rectY, thickness);
			
			// assemble the left dcolumn even part, (0,0) is at the top right of the small rect
			rectdColEvenLeftPart->AddNode(rectSmallRect, 1, new TGeoTranslation(-rectX/2.0, -rectY, 0.0));
			rectdColEvenLeftPart->AddNode(rectRect, 1, new TGeoTranslation(-2.0*rectX, -rectY, 0.0));
			rectdColEvenLeftPart->AddNode(rectRect, 2, new TGeoTranslation(-4.0*rectX, -rectY, 0.0));
			
			// assemble the left dcolumn odd part, (0,0) is at the top right of the small rect
			rectdColOddLeftPart->AddNode(rectSmallRect, 1, new TGeoTranslation(-rectX/2.0, -rectY, 0.0));
			rectdColOddLeftPart->AddNode(rectRect, 1, new TGeoTranslation(-2.0*rectX, -rectY, 0.0));
			
			// assemble the left dcolumn even and odd from the part
			for (int i = 0; i < rectRows; i++){
				rectdColEvenLeft->AddNode(rectdColEvenLeftPart, i+1, new TGeoTranslation(0.0, -i*2.0*rectY, 0.0));
				rectdColOddLeft->AddNode(rectdColOddLeftPart, i+1, new TGeoTranslation(0.0, -i*2.0*rectY, 0.0));
			}
			
			// assemble the dcolumns
			rectdColEven->AddNode(rectdColEvenLeft, 1, new TGeoTranslation(0.0, 0.0, 0.0));
			rectdColEven->AddNode(rectdColEvenLeft, 2, new TGeoCombiTrans(0.0, 0.0, 0.0, rotY180));
			rectdColOdd->AddNode(rectdColOddLeft, 1, new TGeoTranslation(0.0, 0.0, 0.0));
			rectdColOdd->AddNode(rectdColOddLeft, 2, new TGeoCombiTrans(0.0, 0.0, 0.0, rotY180));
			
			// assemble the rect grid
			double rectXShift = 8.0*rectX;
			double rectColumnYCentering = rectRows*rectY;
			rectPart->AddNode(rectdColEvenLeft, 1, new TGeoCombiTrans(-3.0*rectXShift, rectColumnYCentering, 0.0, rotY180));
			rectPart->AddNode(rectdColOdd, 1, new TGeoTranslation(-2.0*rectXShift, rectColumnYCentering, 0.0));
			rectPart->AddNode(rectdColEven, 1, new TGeoTranslation(-rectXShift, rectColumnYCentering, 0.0));
			rectPart->AddNode(rectdColOdd, 2, new TGeoTranslation(0.0, rectColumnYCentering, 0.0));
			rectPart->AddNode(rectdColEven, 2, new TGeoTranslation(rectXShift, rectColumnYCentering, 0.0));
			rectPart->AddNode(rectdColOdd, 3, new TGeoTranslation(2.0*rectXShift, rectColumnYCentering, 0.0));
			rectPart->AddNode(rectdColEvenLeft, 2, new TGeoTranslation(3.0*rectXShift, rectColumnYCentering, 0.0));
			
			//__________________________________________________________________________________________________________
			// hex grid parts
			// double columns even and odd, odd has the extra hex cell in the middle
			TGeoVolume *dColEven = new TGeoVolumeAssembly("dColEven");
			TGeoVolume *dColOdd = new TGeoVolumeAssembly("dColOdd");
			// split double columns in left and right, right is constructed by rotating left by 180 degrees around y axis
			TGeoVolume *dColEvenLeft = new TGeoVolumeAssembly("dColEvenLeft");
			//TGeoVolume *dcolEvenRight = new TGeoVolumeAssembly("dColEvenRight");
			
			TGeoVolume *dColOddLeft = new TGeoVolumeAssembly("dColOddLeft");
			//TGeoVolume *dcolOddRight = new TGeoVolumeAssembly("dColOddRight");
			// define building parts of the columns respectivly, this are left parts, rotate for right parts later
			TGeoVolume *dColEvenLeftPart = new TGeoVolumeAssembly("dColEvenPart");
			TGeoVolume *dColOddLeftPart = new TGeoVolumeAssembly("dColOddPart");
			// last row is a special case
			TGeoVolume *dColEvenLeftPartLast = new TGeoVolumeAssembly("dColEvenPartLast");
			TGeoVolume *dColOddLeftPartLast = new TGeoVolumeAssembly("dColOddPartLast");
			
			// parts of the column part
			// small rectangle
			TGeoVolume *hexSmallRect = _tGeoManager->MakeBox("Rect", Dia, hexRectSmallX, hexRectSmallY, thickness);
			// bigger small rectangle for the last row
			TGeoVolume *hexSmallRectLast = _tGeoManager->MakeBox("RectLast", Dia, hexRectSmallX, hexRectSmallY+pixelY/2.0, thickness);
			
			// hexagon, (0,0) between the two flatHex
			TGeoVolume *hex = new TGeoVolumeAssembly("Hex");
			// flatt half hexagon for easier representation in TGeo, rotate by 90 degree around y axis and shift by hexSide/2. in x, and add a second on with "opposite" transformation to get a full hexagon
			TGeoVolume *flattHex = _tGeoManager->MakeTrd2("FlattHex", Dia, thickness, thickness, 2.0*hexSideY, hexSideY, hexSideX);
			hex->AddNode(flattHex, 1, new TGeoCombiTrans(-hexSideX, 0.0, 0.0, rotY270));
			hex->AddNode(flattHex, 2, new TGeoCombiTrans(hexSideX, 0.0, 0.0, rotY90));
			
			//half flat hex for the last row, and longer by half a FE-I4 pixel y, (0,0) is at the center of the left side befor elonging and the middle of x
			TGeoVolume *hexHalf = new TGeoVolumeAssembly("HexHalf");
			TGeoVolume *flattHexHalf = _tGeoManager->MakeArb8("FlattHexHalf", Dia, thickness);
			TGeoArb8 *flattHexHalfConstr = (TGeoArb8*)flattHexHalf->GetShape();
			flattHexHalfConstr->SetVertex(0, -hexSideX, -hexRectSmallY-pixelY);
			flattHexHalfConstr->SetVertex(1, -hexSideX, hexRectSmallY);
			flattHexHalfConstr->SetVertex(2, hexSideX, 2.0*hexSideY);
			flattHexHalfConstr->SetVertex(3, hexSideX, -hexRectSmallY-pixelY);
			flattHexHalfConstr->SetVertex(4, -hexSideX, -hexRectSmallY-pixelY);
			flattHexHalfConstr->SetVertex(5, -hexSideX, hexRectSmallY);
			flattHexHalfConstr->SetVertex(6, hexSideX, 2.0*hexSideY);
			flattHexHalfConstr->SetVertex(7, hexSideX, -hexRectSmallY-pixelY);
			
			hexHalf->AddNode(flattHexHalf, 1, new TGeoTranslation(-hexSideX, 0.0, 0.0));
			hexHalf->AddNode(flattHexHalf, 2, new TGeoCombiTrans(hexSideX, 0.0, 0.0, rotY180));
			
			// half hex plus rectangle, (0,0) in the middle of the connection of hex and rect
			TGeoVolume *hexRectHex = new TGeoVolumeAssembly("RectHex");
			TGeoVolume *hexRectHexBox = _tGeoManager->MakeBox("RectHexBox", Dia, hexRectBigX, hexRectBigY, thickness);
			hexRectHex->AddNode(hexRectHexBox, 1, new TGeoTranslation(hexRectBigX, 0.0, 0.0));
			hexRectHex->AddNode(flattHex, 1, new TGeoCombiTrans(-hexSideX, 0.0, 0.0, rotY270));
			
			// assemble the left dcolumn even part
			// start with small rect because it has a defined middle which then is (0,0)
			dColEvenLeftPart->AddNode(hexSmallRect, 1, new TGeoTranslation(0.0, 0.0, 0.0));
			// add hexRect
			dColEvenLeftPart->AddNode(hexRectHex, 1, new TGeoTranslation(-hexRectBigX, hexRectSmallY+2.0*hexSideY, 0.0)); //-2.0*hexRectBigX, -hexRectBigY, 0.0));
			// add the first hex left of the hexRect
			dColEvenLeftPart->AddNode(hex, 1, new TGeoTranslation(-4.0*hexSideX-hexRectSmallX, hexRectSmallY+2.0*hexSideY, 0.0));
			// add second hex left of small rect
			dColEvenLeftPart->AddNode(hex, 2, new TGeoTranslation(-2.0*hexSideX-hexRectSmallX, 0.0, 0.0));
			
			// assemble the last row left dcolumn even part
			// bigger small rect
			dColEvenLeftPartLast->AddNode(hexSmallRectLast, 1, new TGeoTranslation(0.0, 0.0, 0.0));
			// add hexRect
			dColEvenLeftPartLast->AddNode(hexRectHex, 1, new TGeoTranslation(-hexRectBigX, hexRectSmallY+pixelY/2.0+2.0*hexSideY, 0.0)); 
			// add first hex left of hexRect
			dColEvenLeftPartLast->AddNode(hex, 1, new TGeoTranslation(-4.0*hexSideX-hexRectSmallX, hexRectSmallY+pixelY/2.0+2.0*hexSideY, 0.0));
			// add first half hex next to longer small rect
			dColEvenLeftPartLast->AddNode(hexHalf, 1, new TGeoTranslation(-2.0*hexSideX-hexRectSmallX, pixelY/2.0, 0.0));
			
			// assemble the left dcolumn odd part
			// start with small rect because it has a defined middle which then is (0,0)
			dColOddLeftPart->AddNode(hexSmallRect, 1, new TGeoTranslation(0.0, 0.0, 0.0));
			// add hexRect
			dColOddLeftPart->AddNode(hexRectHex, 1, new TGeoTranslation(-hexRectBigX, hexRectSmallY+2.0*hexSideY, 0.0)); //-2.0*hexRectBigX, -hexRectBigY, 0.0));
			// add the first hex left of the hexRect
			dColOddLeftPart->AddNode(hex, 1, new TGeoTranslation(-4.0*hexSideX-hexRectSmallX, hexRectSmallY+2.0*hexSideY, 0.0));
			// add second hex left of small rect
			dColOddLeftPart->AddNode(hex, 2, new TGeoTranslation(-2.0*hexSideX-hexRectSmallX, 0.0, 0.0));
			//add third hex left of second hex
			dColOddLeftPart->AddNode(hex, 3, new TGeoTranslation(-6.0*hexSideX-hexRectSmallX, 0.0, 0.0));
			
			// assemble the last row left dcolumn odd part
			// bigger small rect
			dColOddLeftPartLast->AddNode(hexSmallRectLast, 1, new TGeoTranslation(0.0, 0.0, 0.0));
			// add hexRect
			dColOddLeftPartLast->AddNode(hexRectHex, 1, new TGeoTranslation(-hexRectBigX, hexRectSmallY+pixelY/2.0+2.0*hexSideY, 0.0)); 
			// add first hex left of hexRect
			dColOddLeftPartLast->AddNode(hex, 1, new TGeoTranslation(-4.0*hexSideX-hexRectSmallX, hexRectSmallY+pixelY/2.0+2.0*hexSideY, 0.0));
			// add first half hex next to longer small rect
			dColOddLeftPartLast->AddNode(hexHalf, 1, new TGeoTranslation(-2.0*hexSideX-hexRectSmallX, pixelY/2.0, 0.0));
			// add second half hex next to longer small rect
			dColOddLeftPartLast->AddNode(hexHalf, 1, new TGeoTranslation(-6.0*hexSideX-hexRectSmallX, pixelY/2.0, 0.0));
			
			// assemble the left even and odd dcolumns from the parts
			double yshift = -2.0*hexRectBigY-2.0*hexRectSmallY; // y size of the parts, should be 200mu
			for (int i = 0; i < hexRows-1; i++){
				dColEvenLeft->AddNode(dColEvenLeftPart, i+1, new TGeoTranslation(0.0, i*yshift, 0.0));
				dColOddLeft->AddNode(dColOddLeftPart, i+1, new TGeoTranslation(0.0, i*yshift, 0.0));
			}
			// add the last special row
			dColEvenLeft->AddNode(dColEvenLeftPartLast, 1, new TGeoTranslation(0.0, (hexRows-1)*yshift-pixelY/2.0, 0.0));
			dColOddLeft->AddNode(dColOddLeftPartLast, 1, new TGeoTranslation(0.0, (hexRows-1)*yshift-pixelY/2.0, 0.0));
			// assemble the dcolumns
			double hexColumnYCentering = hexRows*2.0*(hexRectBigY+hexRectSmallY)/2.0+pixelY/2.0-hexRectSmallY-2.0*hexRectBigY-pixelY/2.0;
			dColEven->AddNode(dColEvenLeft, 1, new TGeoTranslation(-hexRectSmallX, hexColumnYCentering, 0.0)); //-hexRectSmallX-1.5*hexSide*std::sqrt(3.0)
			dColEven->AddNode(dColEvenLeft, 2, new TGeoCombiTrans(hexRectSmallX, hexColumnYCentering, 0.0, rotY180)); //hexRectSmallX+1.5*hexSide*std::sqrt(3.0)
			dColOdd->AddNode(dColOddLeft, 1, new TGeoTranslation(-hexRectSmallX, hexColumnYCentering, 0.0)); //-hexRectSmallX-2.0*hexSide*std::sqrt(3.0)
			dColOdd->AddNode(dColOddLeft, 2, new TGeoCombiTrans(hexRectSmallX, hexColumnYCentering, 0.0, rotY180)); //hexRectSmallX+2.0*hexSide*std::sqrt(3.0)
			
			// assemble the hex grid, x = 0 on the left edge
			double hexXShift = 4.0*hexRectBigX+12.0*hexSideX; // should be 500mu
			hexPart->AddNode(dColEvenLeft, 1, new TGeoCombiTrans(-3.0*hexXShift+hexRectSmallX, hexColumnYCentering, 0.0, rotY180));
			hexPart->AddNode(dColOdd, 1, new TGeoTranslation(-2.0*hexXShift, 0.0, 0.0));
			hexPart->AddNode(dColEven, 1, new TGeoTranslation(-hexXShift, 0.0, 0.0));
			hexPart->AddNode(dColOdd, 2, new TGeoTranslation(0.0, 0.0, 0.0));
			hexPart->AddNode(dColEven, 2, new TGeoTranslation(hexXShift, 0.0, 0.0));
			hexPart->AddNode(dColOdd, 3, new TGeoTranslation(2.0*hexXShift, 0.0, 0.0));
			hexPart->AddNode(dColEvenLeft, 2, new TGeoTranslation(3.0*hexXShift-hexRectSmallX, hexColumnYCentering, 0.0));
			
			//______________________________________________________________________________________________________________
			// assemble the non metalised regions also called free
			TGeoVolume *pixel = _tGeoManager->MakeBox("Pixel", Dia, pixelX, pixelY, thickness);
			// left free
			TGeoVolume *leftColumn = new TGeoVolumeAssembly("LeftCol");
			for (int i = 0; i < leftRows; i++){
				leftColumn->AddNode(pixel, i+1, new TGeoTranslation(0.0, -i*2.0*pixelY, 0.0));
			}
			leftFree->AddNode(leftColumn, 1, new TGeoTranslation(-2.0*pixelX, -pixelY+leftRows*pixelY, 0.0));
			leftFree->AddNode(leftColumn, 2, new TGeoTranslation(0.0, -pixelY+leftRows*pixelY, 0.0));
			leftFree->AddNode(leftColumn, 3, new TGeoTranslation(2.0*pixelX, -pixelY+leftRows*pixelY, 0.0));
			
			// top free
			TGeoVolume *topColumn = new TGeoVolumeAssembly("TopCol");
			for (int i = 0; i < topRows; i++){
				topColumn->AddNode(pixel, i+1, new TGeoTranslation(0.0, -i*2.0*pixelY, 0.0));
			}
			for (int i = 0; i < midCols/2.0; i++){
				topFree->AddNode(topColumn, midCols/2.0-i, new TGeoTranslation(-2.0*i*pixelX-pixelX, -pixelY+topRows*pixelY, 0.0));
				topFree->AddNode(topColumn, midCols/2.0+i+1, new TGeoTranslation(2.0*i*pixelX+pixelX, -pixelY+topRows*pixelY, 0.0));
			}
			
			// bot free
			TGeoVolume *botColumn = new TGeoVolumeAssembly("BotCol");
			for (int i = 0; i < botRows; i++){
				botColumn->AddNode(pixel, i+1, new TGeoTranslation(0.0, -i*2.0*pixelY, 0.0));
			}
			for (int i = 0; i < midCols/2.0; i++){
				botFree->AddNode(botColumn, midCols/2.0-i, new TGeoTranslation(-2.0*i*pixelX-pixelX, -pixelY+botRows*pixelY, 0.0));
				botFree->AddNode(botColumn, midCols/2.0+i+1, new TGeoTranslation(2.0*i*pixelX+pixelX, -pixelY+botRows*pixelY, 0.0));
			}
			
			// right free
			TGeoVolume *rightColumn = new TGeoVolumeAssembly("RightCol");
			for (int i = 0; i < rightRows; i++){
				rightColumn->AddNode(pixel, i+1, new TGeoTranslation(0.0, -i*2.0*pixelY, 0.0));
			}
			rightFree->AddNode(rightColumn, 1, new TGeoTranslation(-2.0*pixelX, -pixelY+rightRows*pixelY, 0.0));
			rightFree->AddNode(rightColumn, 2, new TGeoTranslation(0.0, -pixelY+rightRows*pixelY, 0.0));
			rightFree->AddNode(rightColumn, 3, new TGeoTranslation(2.0*pixelX, -pixelY+rightRows*pixelY, 0.0));
			
			//______________________________________________________________________________________________________________
			// assemble the whole layout
			plane->AddNode(leftFree, 1, new TGeoTranslation(-xMid-xLeft, 0.0, 0.0));
			plane->AddNode(topFree, 1, new TGeoTranslation(0.0, 2.0*yRectPart+yTop+7.0*pixelY, 0.0)); // 7*pixelY because the change in pixel is not in the middle because on top not all the diamond is on the FE pixel
			plane->AddNode(rectPart, 1, new TGeoTranslation(0.0, yRectPart+7.0*pixelY, 0.0));
			plane->AddNode(hexPart, 1, new TGeoTranslation(0.0, -yHexPart+7.0*pixelY, 0.0));
			plane->AddNode(botFree, 1, new TGeoTranslation(0.0, -2.0*yHexPart-yBot+7.0*pixelY, 0.0));
			plane->AddNode(rightFree, 1, new TGeoTranslation(xMid+xRight, 0.0, 0.0));
			
		}

		FEI4Diamond::~FEI4Diamond() {
					// delete matSi;
					// delete Si;
		}
		void FEI4Diamond::createRootDescr(char const *planeVolume) {
			// Get the plane as provided by the EUTelGeometryTelescopeGeoDescription
			TGeoVolume *topplane = _tGeoManager->GetVolume(planeVolume);
			// Finaly add the sensitive area to the plane
			topplane->AddNode(plane, 1);
		}

		std::string FEI4Diamond::getPixName(int x, int y) {
			//char buffer[100];
			std::string buffer = "";
			// since pixel 0|0 is located on the upper left corner we have to correct
			// y by 335-y+1 // TODO do I need this shift?
			//(one for the offset in TGeo which starts counting at 1)
			
			buffer = "/FEI4Dia_1"; // base path
			
			// now match the FE-I4 pixel coordinates x and y to the different pixels in the Layout
			
			// no metallisation
			if (x >= layoutStartX && x < metalStartX && y >= layoutStartY && y <= layoutEndY){ // leftFree
				//x-layoutStartX+1,-y+1+335
				buffer += "/LeftFree_1/LeftCol_" + std::to_string(x-layoutStartX+1) + "/Pixel_" + std::to_string(y+1);
				return buffer;
			}
			else if (x >= metalStartX && x < metalEndX && y >= layoutStartY && y < metalStartY){ // topFree
				buffer += "/TopFree_1/TopCol_" + std::to_string(x-metalStartX+1) + "/Pixel_" + std::to_string(y+1);
				return buffer;
			}
			else if (x >= metalStartX && x < metalEndX && y >= metalEndY && y <= layoutEndY){ // botFree
				buffer += "/BotFree_1/BotCol_" + std::to_string(x-metalStartX+1) + "/Pixel_" + std::to_string(y-metalEndY+1);
				return buffer;
			}
			else if (x >= metalEndX && x <= layoutEndX && y >= layoutStartY && y <= layoutEndY){ // rightFree
				buffer += "/RightFree_1/RightCol_" + std::to_string(x-metalEndX+1) + "/Pixel_" + std::to_string(y+1);
				return buffer;
			}
			
			//rectangular metallisation
			else if (x >= metalStartX && x < metalEndX && y >= metalStartY && y < hexStartY){ 
				buffer += "/RectPart_1";
				if (x == metalStartX){
					buffer += "/rectdColEvenLeft_1/rectdColEvenPart_";
					evendCol = true;
				}
				else if (x == metalEndX-1){
					buffer += "/rectdColEvenLeft_2/rectdColEvenPart_";
					evendCol = true;
				}
				else if (((x - metalStartX) % 2) == 0){ // even number of FE column because shift of 1
					if ((((x - metalStartX) / 2) % 2) == 0){ // even number double column
						buffer += "/rectdColEven_" + std::to_string((x-metalStartX)/2/2);
						buffer += "/rectdColEvenLeft_2/rectdColEvenPart_";
						evendCol = true;
					}
					else { // odd number double column
						buffer += "/rectdColOdd_" + std::to_string((x-metalStartX+2)/2/2);
						buffer += "/rectdColOddLeft_2/rectdColOddPart_";
						evendCol = false;
					}
				}
				else { // odd number of FE column
					if ((((x - metalStartX + 1) / 2) % 2) == 0){ // even number double column
						buffer += "/rectdColEven_" + std::to_string((x-metalStartX+1)/2/2);
						buffer += "/rectdColEvenLeft_1/rectdColEvenPart_";
						evendCol = true;
					}
					else { // odd number double column
						buffer += "/rectdColOdd_" + std::to_string((x-metalStartX+1+2)/2/2);
						buffer += "/rectdColOddLeft_1/rectdColOddPart_";
						evendCol = false;
					}
				}
				
				if ((y - metalStartY) % 2 == 0){
					buffer += std::to_string((y - metalStartY) / 2 + 1);
					if (evendCol){
						buffer += "/RectRect_1";
					}
					else {
						buffer += "/SmallRect_1";
					}
				}
				else {
					buffer += std::to_string((y - metalStartY - 1) / 2 + 1);
					if (evendCol){
						buffer += "/RectRect_2";
					}
					else {
						buffer += "/RectRect_1";
					}
				}
				return buffer;
			} // rectangular metallisation
			
			// hexagonal metallisation
			else if (x >= metalStartX && x < metalEndX && y >= hexStartY && y < metalEndY){
				buffer += "/HexPart_1";
				if (x == metalStartX){
					buffer += "/dColEvenLeft_1/dColEvenPart";
					evendCol = true;
				}
				else if (x == metalEndX - 1){
					buffer += "/dColEvenLeft_2/dColEvenPart";
					evendCol = true;
				}
				
				else if (((x - metalStartX) % 2) == 0){ // even number of FE column because shift of 1
					if ((((x - metalStartX) / 2) % 2) == 0){ // even number double column
						buffer += "/dColEven_" + std::to_string((x-metalStartX)/2/2);
						buffer += "/dColEvenLeft_2/dColEvenPart";
						evendCol = true;
					}
					else { // odd number double column
						buffer += "/dColOdd_" + std::to_string((x-metalStartX+2)/2/2);
						buffer += "/dColOddLeft_2/dColOddPart";
						evendCol = false;
					}
				}
				else { // odd number of column
					if ((((x - metalStartX + 1) / 2) % 2) == 0){ // even number double column
						buffer += "/dColEven_" + std::to_string((x-metalStartX+1)/2/2);
						buffer += "/dColEvenLeft_1/dColEvenPart";
						evendCol = true;
					}
					else { // odd number double column
						buffer += "/dColOdd_" + std::to_string((x-metalStartX+1+2)/2/2);
						buffer += "/dColOddLeft_1/dColOddPart";
						evendCol = false;
					}
				}
				// y periodicity but take care for the last part which is longer and special
				if ((y - hexStartY) % 4 == 0){
					if (y == metalEndY - 5){
						buffer += "Last_1/Hex_1";
					}
					else if (y == metalEndY -1){
						buffer += "Last_1/RectLast_1";
					}
					else {
						buffer += "_" + std::to_string((y - hexStartY) / 4 + 1);
						buffer += "/Hex_1";
					}
				}
				else if ((y - hexStartY) % 4 == 1){
					if (y == metalEndY - 4){
						buffer += "Last_1";
					}
					else {
						buffer += "_" + std::to_string((y - hexStartY - 1) / 4 + 1);
					}
					buffer += "/RectHex_1";
				}
				else if ((y - hexStartY) % 4 == 2){
					if (y == metalEndY - 3){
						buffer += "Last_1/RectHex_1";
					}
					else {
						buffer += "_" + std::to_string((y - hexStartY - 2) / 4 + 1);
						buffer += "/Hex_2";
					}
				}
				else { // remainder 3
					if (y == metalEndY - 2){
						buffer += "Last_1/RectLast_1";
					}
					else {
						buffer += "_" + std::to_string((y - hexStartY - 3) / 4 + 1);
						if (evendCol){
							buffer += "/Rect_1";
						}
						else {
							buffer += "/Hex_3";
						}
					}
				}
				return buffer;
				
			} // hexagonal metallisation
			// Pixel hit outside of the diamond in the rest of the FE-I4 not handled , needed?
			return "";
		}

		// TODO: parse the path to a pixel number!
		std::pair<int, int> FEI4Diamond::getPixIndex(char const *) {
			return std::make_pair(0, 0);
		}

		EUTelGenericPixGeoDescr *maker() {
			FEI4Diamond *mPixGeoDescr = new FEI4Diamond();
			return dynamic_cast<EUTelGenericPixGeoDescr *>(mPixGeoDescr);
		}

	} // namespace geo
} // namespace eutelescope
