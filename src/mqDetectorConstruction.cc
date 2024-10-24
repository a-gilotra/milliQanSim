// created 22-04-2019
// author: Ryan Schmitz (UCSB)

//==============================================================================
#include "mqDetectorConstruction.hh"
#include "mqScintSD.hh"
#include "mqPMTSD.hh"
#include "mqBarParameterisation.hh"
#include "mqPMTParameterisation.hh"
#include "mqUserEventInformation.hh"

#include "MilliQMonopoleFieldSetup.hh"

#include <sstream>
#include <math.h>
#include "globals.hh"
// Includes Physical Constants and System of Units
#include "G4PhysicalConstants.hh"
#include "G4SystemOfUnits.hh"
#include "G4UnitsTable.hh"
#include "G4ios.hh"
#include "G4VUserDetectorConstruction.hh"
#include "G4ThreeVector.hh"
#include "G4RotationMatrix.hh"
#include "G4Trd.hh"
#include "G4Material.hh"
#include "G4NistManager.hh"
#include "G4RunManager.hh"
#include "G4UserLimits.hh"
#include "G4VisAttributes.hh"
#include "G4Colour.hh"
#include "G4EmCalculator.hh"
#include "G4ParticleDefinition.hh"
#include "G4MuonMinus.hh"
#include "G4MuonPlus.hh"

#include "G4MultiUnion.hh"
#include "G4Box.hh"
#include "G4Tubs.hh"
#include "G4EllipticalTube.hh"
#include "G4Trap.hh"
#include "G4Polycone.hh"
#include "G4Polyhedra.hh"
#include "G4UnionSolid.hh"
#include "G4SubtractionSolid.hh"

#include "G4RunManager.hh"
#include "G4LogicalBorderSurface.hh"
#include "G4LogicalSkinSurface.hh"
#include "G4OpticalSurface.hh"
#include "G4MaterialTable.hh"
#include "G4NistManager.hh"
#include "G4PhysicsVector.hh"

#include "G4Sphere.hh"
#include "G4LogicalVolume.hh"
#include "G4LogicalVolume.hh"

#include "G4PVPlacement.hh"
#include "G4VPVParameterisation.hh"
#include "G4PVParameterised.hh"
#include "globals.hh"
#include "G4SolidStore.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4PhysicalVolumeStore.hh"
#include "G4GeometryManager.hh"

#include "G4LogicalVolume.hh"
#include "G4VPhysicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4AssemblyVolume.hh"


#include "G4OpticalSurface.hh"
#include "G4LogicalBorderSurface.hh"
#include "G4SDManager.hh"
#include "G4UImanager.hh"

const G4int nLayers = 4; //number of layers in detector. Up here since it defines an array


//const G4String mqOpticalFilePath = "/home/users/ryan/milliQanDemoSim/OpticalData/";
//const G4String mqOpticalFilePath = (std::string)std::getenv("PROJECTDIR")+"/OpticalData/";
const G4String mqOpticalFilePath = "../OpticalData/";
//==============================================================================
mqDetectorConstruction::mqDetectorConstruction() :
    verbose(1)//,solidWorld(0), logicWorld(0), physicWorld(0)
{
	SetDefaults();
}

//==============================================================================
mqDetectorConstruction::~mqDetectorConstruction() {

}
//==============================================================================

//==============================================================================

G4VPhysicalVolume* mqDetectorConstruction::Construct() {

	return SetupGeometry();
}

//==============================================================================

G4VPhysicalVolume* mqDetectorConstruction::SetupGeometry() {

	if (verbose >= 0) {
		G4cout << "MilliQan> Construct geometry." << G4endl;
	}
	G4double fWorld_x = 20.0 * m;
	G4double fWorld_y = 20.0 * m;
	G4double fWorld_z = 20.0 * m;

	G4Material* worldMaterial = G4NistManager::Instance()->FindOrBuildMaterial(
	//			"Galactic");
				"G4_AIR");

	const G4int nEntriesAir = 2;
	G4double photonEnergyAir[nEntriesAir]={ 6.3 * eV,1.5 * eV};
	G4double Air_RIND[nEntriesAir];

	for (int i = 0; i < nEntriesAir; i++) {
		Air_RIND[i]= 1.0;
	}// max value at 440 nm

	G4MaterialPropertiesTable* mptAir = new G4MaterialPropertiesTable();
	mptAir->AddProperty("RINDEX",photonEnergyAir, Air_RIND, nEntriesAir);//->SetSpline(true);
	worldMaterial->SetMaterialPropertiesTable(mptAir);

	G4Box* solidWorld = new G4Box("world", fWorld_x / 2, fWorld_y / 2, fWorld_z / 2);
	G4LogicalVolume* logicWorld = new G4LogicalVolume(solidWorld, worldMaterial, "World", 0, 0,
			0);

	G4VisAttributes* visAttWorld = new G4VisAttributes();
	visAttWorld->SetVisibility(false);
	logicWorld->SetVisAttributes(visAttWorld);

	//Must place the World Physical volume unrotated at (0,0,0).
	G4PVPlacement* physicWorld = new G4PVPlacement(0, // no rotation
			G4ThreeVector(), // at (0,0,0)
			logicWorld, // its logical volume
		"World", // its name
			0, // its mother  volume
			false, // no boolean operations
			0); // copy number

//// initialize magnetic field ////
/* //disabling for now since it made no difference and it breaks some builds
      	fMonFieldSetup = MilliQMonopoleFieldSetup::GetMonopoleFieldSetup();
	G4double fieldValueX = -1.61*gauss;		//-14.00*gauss;
	G4double fieldValueY = -1.73*gauss;		//12.63*gauss;
	G4double fieldValueZ = 21.57*gauss;		//10.74*gauss;
	//total magnitude: 21.7 gauss
	this->SetMagField(fieldValueX, fieldValueY, fieldValueZ);
	//0 is standard field, 1 is field which works for monopoles
	fMonFieldSetup->SetStepperAndChordFinder(1);
*/
///////////////////////////////    End world //////////////////////////////////////////////////
///////////////////////////////   Start Scintillator/Wrapping  ///////////////////////////////////////////
	//==============================================================================
	// size of Scintillator and wrapping
	//==============================================================================

	G4double measurementPos=243*mm;
	G4double wrapRefl = 0.97;//0.97;
	G4double scintX = 50/2*mm;
	G4double scintY = 50/2*mm; 
	G4double scintXY = 50/2*mm;
	G4double scintZ  = 800/2*mm;//(each of these dimensions represents the half-width; e.g. this is 600mm long)

	G4double airGapThickness = 0.05*cm; //0.2*mm; //1*mm
	G4double wrapThickness = 0.05*cm; //0.15*mm; //3*mm
	G4double airgapX = scintX+airGapThickness;	
	G4double airgapY = scintY+airGapThickness;	
	G4double airgapXY = scintXY+airGapThickness;	
	G4double airgapZ = scintZ+airGapThickness;
	G4double wrapX=airgapX+wrapThickness;
	G4double wrapY=airgapY+wrapThickness;
	G4double wrapXY=airgapXY+wrapThickness;
	G4double wrapZ=airgapZ+wrapThickness;

        G4double frontLayerMid = -135*cm;
        G4double midLayerMid = -10*cm;
        G4double backLayerMid = 120*cm;
	G4double fourthLayerMid = 250*cm;

	//this is here because the entire geometry is shifted 6cm upwards
        G4double centerOffsetX = 6*cm;
	G4double centerOffsetY = 0*cm;

	//measured from the centers; the gap between the bars
	G4double barSpacingXY = 60*mm;
	//G4double layerSpacing = 1000*mm; 
	//in visualization this gets swapped, so this is actually 3x2 rather than 2x3
        G4int nBarXCount = 2; //number of bars in grid, so this is NxN
        G4int nBarYCount = 2; //number of bars in grid, so this is NxN
	this->SetNBarPerLayer(nBarXCount*nBarYCount);
	this->SetNLayer(nLayers);
 
	 G4double outerRadius_pmt   = 51* mm /2.; //26 //52
	 G4double outerRadius_cath  = 46* mm /2.; //26 //52
	 G4double height_pmt        = 147* mm; //142.*mm;
	 G4double height_cath       = 2.0 * mm; //2mm
	 G4double muMetalThickness = 0.5*mm;

	 G4double ScintSlabX = 31/2*cm;
	 G4double ScintSlabY = 20.5/2*cm;
	 G4double ScintSlabZ = 2.5/2*cm;
	 G4double ScintSlabWrapX = ScintSlabX+airGapThickness+wrapThickness;
	 G4double ScintSlabWrapY = ScintSlabY+airGapThickness+wrapThickness;
	 G4double ScintSlabWrapZ = ScintSlabZ+airGapThickness+wrapThickness;
	 G4double ScintSlabAirGapX = ScintSlabX+airGapThickness;
	 G4double ScintSlabAirGapY = ScintSlabY+airGapThickness;
	 G4double ScintSlabAirGapZ = ScintSlabZ+airGapThickness;
	 G4double ScintPassSlabX = 1*cm/2;
	 G4double ScintPassSlabY = ScintSlabWrapY*2+1*cm;
	 G4double ScintPassSlabZ = ScintSlabWrapX*2;

	 G4double ScintSlabOffsetX = 7.5*cm;
	 G4double ScintSlabOffsetZ0 = -scintZ-ScintSlabZ+6*cm; //scintZ-2*cm //-174*cm
	 G4double ScintSlabOffsetZ1 = -scintZ-ScintSlabZ-1*cm; //scintZ-3*cm //-53*cm
	 G4double ScintSlabOffsetZ2 = -scintZ-ScintSlabZ-4*cm; //scintZ-2.5*cm //72.5*cm;
	 //G4double ScintSlabOffsetZ0 = -scintZ-ScintSlabZ+1*cm; //scintZ-2*cm //-174*cm
	 //G4double ScintSlabOffsetZ1 = -scintZ-ScintSlabZ-6*cm; //scintZ-3*cm //-53*cm
	 //G4double ScintSlabOffsetZ2 = -scintZ-ScintSlabZ-9*cm; //scintZ-2.5*cm //72.5*cm;
	 G4double ScintSlabOffsetZEnd = scintZ+height_pmt+ScintSlabZ-12*cm; //scintZ-2.5*cm //181.5
	 
	 G4double ScintPanelX = 0.274*2.54/2*cm;
	 G4double ScintPanelY = 7*2.54/2*cm;
	 G4double ScintPanelZ = 40*2.54/2*cm;
	 G4double ScintPanelWrapX = ScintPanelX+airGapThickness+wrapThickness;
	 G4double ScintPanelWrapY = ScintPanelY+airGapThickness+wrapThickness;
	 G4double ScintPanelWrapZ = ScintPanelZ+airGapThickness+wrapThickness;
	 G4double ScintPanelAirGapX = ScintPanelX+airGapThickness;
	 G4double ScintPanelAirGapY = ScintPanelY+airGapThickness;
	 G4double ScintPanelAirGapZ = ScintPanelZ+airGapThickness;
	 
	G4double ScintPanelOffsetXTop = centerOffsetX+barSpacingXY*(nBarXCount-1)/2+scintX+1.5*cm; //16*cm
	 G4double ScintPanelOffsetXLeft = 6.65*cm;
	 G4double ScintPanelOffsetXRight = 6.65*cm;
	 G4double ScintPanelOffsetYRight = barSpacingXY*(nBarYCount-1)/2+scintY+3*cm;
	 G4double ScintPanelOffsetYLeft = -ScintPanelOffsetYRight;
	 
	 G4double pmtOffsetY = ScintSlabY/2;//0*cm;//ScintSlabY-outerRadius_pmt;

	G4double HodoX = 8.6*cm/2; //width
	G4double HodoY = 45*cm/2; //length //horizontal bars by default
	G4double HodoZ = 4.2*cm/2; //thickness

	G4double HodoWrapX = HodoX+wrapThickness;
	G4double HodoWrapY = HodoY+wrapThickness;
	G4double HodoWrapZ = HodoZ+wrapThickness;

	G4double HodoOffsetVertYRight = 4.8*cm;
	G4double HodoOffsetVertYLeft = -HodoOffsetVertYRight;
	G4double HodoOffsetVertX = -2.5*cm;

	G4double HodoOffsetHorXUpper = 10.4*cm;
	G4double HodoOffsetHorXLower = 0.8*cm;
	G4double HodoOffsetHorY = 0*cm;

	G4double HodoOffsetVertZFront = ScintSlabOffsetZ0+frontLayerMid-8.6*cm;
	G4double HodoOffsetHorZFront = ScintSlabOffsetZ0+frontLayerMid-3.9*cm;
	G4double HodoOffsetHorZBack = ScintSlabOffsetZEnd+backLayerMid+5.6*cm;
	G4double HodoOffsetVertZBack = ScintSlabOffsetZEnd+backLayerMid+11.1*cm;

	//controls rotation of detector w.r.t. ground 
	G4double worldRotation = -43.1*deg;
	// G4double worldRotation = 0*deg;

	 G4double LeadX = 20/2*cm;
	 G4double LeadY = 20/2*cm;
	 G4double LeadZ = 5/2*cm;
	 G4double LeadOffsetX = 5*cm;
	 G4double LeadOffsetZ = ScintSlabOffsetZ1-LeadZ-1.5*cm; //same for both Z1,Z2 for LeadShield
	
	 G4double AlSupportX     = 10*cm/2;
         G4double AlSupportY     = 15*cm/2;
         G4double AlSupportZ     = (backLayerMid-frontLayerMid+height_pmt+scintZ*2+13*cm)/2; //360*cm
	 G4double AlSupportXOffset = ScintSlabOffsetX-ScintSlabX-AlSupportX;

	 G4double wallThickness = 1*m; //using 2m before
	 G4double wallCylRadius = 1.45*m;//1.8*m;
	 G4double wallCylRadiusOut = wallCylRadius+wallThickness;
	 G4double wallZ = 15*m/2;
         
	 G4double floorCutoutDepthX = (14*cm+(7*2.54*cm-5*cm)+wallCylRadius)/2;
         G4double floorCutoutDepthY = (-20*cm+wallCylRadius);
	 
	 G4double floorZ = wallZ;

	 G4double overallDetX = ScintPanelOffsetXTop+ScintPanelX+120*cm; //adding larger buffer to get volume above and below
	 G4double overallDetY = ScintPanelOffsetYRight+140*cm; //adding larger buffer since the volumes are angled, PMT is extending, etc.
	 G4double overallDetZ = backLayerMid+ScintSlabOffsetZEnd+ScintSlabWrapZ+122*cm; //adding 5*mm buffer

	 //todo: modify this bool to be able to adjust from single bar mode to demonstrator mode
	//(together with modifying number of layers and bars, etc.)
	 bool SupportStructure = true;
	 G4double AlThickness = 1*cm;

	 G4double SteelThickness = 1*cm;
	 G4double SteelSideLength = 1*m/2;

	 G4double SteelSupportOffsetX = -0.7*m;
	 G4double SteelSupportOffsetZ = 0.8*m;

	G4NistManager* nistMan = G4NistManager::Instance();

///////////////////////////////////////////////////////////////////////////////
	//==========================================================================
	//Scintillator Materials
	//==========================================================================

	//==========================================================================
	// Elements
	//
	//*******************************************************************************************************************************
	G4Element* elH = nistMan->FindOrBuildElement("H");
	G4Element* elC = nistMan->FindOrBuildElement("C");
	G4Element* elO = nistMan->FindOrBuildElement("O");
	G4Element* elK = nistMan->FindOrBuildElement("K");
	G4Element* elNa = nistMan->FindOrBuildElement("Na");
	G4Element* elSi = nistMan->FindOrBuildElement("Si");
	G4Element* elAl = nistMan->FindOrBuildElement("Al");
        G4Element* elCs = nistMan->FindOrBuildElement("Cs"); // Cesium;
        G4Element* elSb = nistMan->FindOrBuildElement("Sb"); // Antimony;
        G4Element* elB = nistMan->FindOrBuildElement("B"); // Boron;
	
	//mu metal materials	
	G4Element* elNi = nistMan->FindOrBuildElement("Ni"); //Nickel;
	G4Element* elFe = nistMan->FindOrBuildElement("Fe"); //Iron;
	
	//there are other parts, but to first order this is 80% Ni, 15% Fe, 5% other stuff, so I'm just going to lump that into Fe
	G4Material* muMetal = new G4Material("muMetal",8.7*g/cm3, 2);
	muMetal->AddElement(elNi,0.8); //80% mass fraction
	muMetal->AddElement(elFe,0.2); //20% mass fraction

	G4Element* elPlastic = nistMan->FindOrBuildElement("G4_POLYSTYRENE");
	//==========================================================================
	// BC-400 plastic scintillator; according to datasheet, elH = 1.103 *elC, elH + elC = 1
	//For the references for the optical properties see ../ref/EmissionSpectrum_BC-400.pdf

	G4Material* matPlScin = new G4Material("plScintillator", 1.032 * g / cm3, 2);
	//matPlScin->AddElement(elC, .4755);
	//matPlScin->AddElement(elH, .5245);
	matPlScin->AddElement(elC, 10);
	matPlScin->AddElement(elH, 11);
	matPlScin->SetMaterialPropertiesTable(SetOpticalPropertiesOfPS());
	matPlScin->GetIonisation()->SetBirksConstant(0.126*mm/MeV); // according to L. Reichhart et al., Phys. Rev, use (0.149*mm/MeV) for neutrons, but otherwise use 0.126
	
	G4Material* matPlScinNoPhoton = new G4Material("plScintillatorNoPhoton", 1.032 * g / cm3, 2);
	//matPlScin->AddElement(elC, .4755);
	//matPlScin->AddElement(elH, .5245);
	matPlScinNoPhoton->AddElement(elC, 10);
	matPlScinNoPhoton->AddElement(elH, 11);
	//==========================================================================
	G4Material* wrapMat = nistMan->FindOrBuildMaterial("G4_POLYETHYLENE");
	//G4Material* airgapMat = worldMaterial; //The air gap mat should be air, change this if you change worldmat to be vacuum/galactic		
	G4Material* AlMat = nistMan->FindOrBuildMaterial("G4_Al");
	G4Material* PbMat = nistMan->FindOrBuildMaterial("G4_Pb");

	G4Material* steelMat = nistMan->FindOrBuildMaterial("G4_STAINLESS-STEEL");
	//going to use concrete as a first order approximation to rock, since it's close and we just want something dense
	G4Material* concreteMat = nistMan->FindOrBuildMaterial("G4_CONCRETE");
	
	G4Material* silicaMat = new G4Material("SiO2", 2.65 * g / cm3, 2);
	silicaMat->AddElement(elSi, 1);
	silicaMat->AddElement(elO, 2);


//Container volume so we can rotate the entire detector at once

	G4RotationMatrix* detRot = new G4RotationMatrix();
	detRot->rotateY(worldRotation);

	G4Box* detectorWorldSolid = new G4Box("detectorWorldSolid",
				overallDetX,
				overallDetY,
				overallDetZ);

	G4LogicalVolume* detectorWorldLogic = new G4LogicalVolume(
				detectorWorldSolid,
				worldMaterial,
				"detectorWorldLogic",
				0,0,0);
/*
	G4PVPlacement* detectorWorldPhysic = new G4PVPlacement(
				detRot,
				G4ThreeVector(0,centerOffsetY,0),
				detectorWorldLogic,
				"detectorWorldPhysic",
				logicWorld,
				false,
				0,
				true);
*/
	G4RotationMatrix* rot = new G4RotationMatrix(); //just an unrotated matrix used where this can't be a 0 arg

////////////////////////////////////////////////////////////////////////////////////////////////////
//Rock to be penetrated by muons as a test. Turn this off by commenting out the placement line below
////////////////////////////////////////////////////////////////////////////////////////////////////
//      G4RotationMatrix* wallRot = new G4RotationMatrix();
//      wallRot->rotateY(43.1*deg);

        G4Tubs* wallsCyl = new G4Tubs("wallsCyl",
                                wallCylRadius, //rMin
                                wallCylRadiusOut, //rMax
                                wallZ, //half-Z
                                0*deg,
                                360*deg);

        G4Box* floorCutout = new G4Box("floorCutout",
                                floorCutoutDepthX,
                                floorCutoutDepthY,
                                floorZ-10*cm);

        G4SubtractionSolid* rockSolid = new G4SubtractionSolid("rockWalls",
                                wallsCyl,
                                floorCutout,
                                rot,
                                G4ThreeVector(-floorCutoutDepthX,0,0));
                                //G4ThreeVector(0,0,0));
///*
        G4LogicalVolume* rockLogic = new G4LogicalVolume(
                                rockSolid,
                                //
                                //silicaMat,
                                concreteMat,
                                //PbMat, //testing muon-induced neutrons
                                "rockLogic",
                                0,0,0);
////////////////////////////////////////////// TURN CAVERN ON/OFF BY COMMENTING OR UNCOMMENTING THIS CODE BLOCK /////////////////////
///*
        G4PVPlacement* rockPhysic = new G4PVPlacement(
                                rot,
                                //wallRot,
                                //G4ThreeVector(50*cm,0,0),//muonLoc: 20*cm+ScintSlabOffsetZEnd+ScintSlabZ+scintZ), //20cm from end of slab, plus add space for slabZ and bar thickness
                                //G4ThreeVector(6*cm,0,0),//6cm is flush. from measurements from Neha and Teresa, shift is 4in
                                //G4ThreeVector(10*cm,0,0),//6cm is flush. from measurements from Neha and Teresa, shift is 4in
                                G4ThreeVector(37.3*cm-6.5*2.54*cm,-23*2.54*cm,0),//37.3cm is flush in X. y offset from neha and teresa
                                //G4ThreeVector(37.3*cm-6.5*2.54*cm,-23*2.54*cm,0),//37.3cm is flush in X. y offset from neha and teresa
                                rockLogic,
                                "rockPhysic",
                                logicWorld,
                                false,
                                0,
                                true);

//*/
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///*	
	G4VisAttributes* rockVis = new G4VisAttributes(
			G4Colour::White());
	rockVis->SetColour(0.64,0.16,0.16,0.2);
	rockVis->SetVisibility(true);
	rockLogic->SetVisAttributes(rockVis);
//*/
/////////////////////////////////////////////////////////////////////////////////
//                            Wrapping Geometry                                //
/////////////////////////////////////////////////////////////////////////////////	
///* //geometry to be used when photocathode is attached to PMT
	G4Box* wrap_solid_total = new G4Box("wrap_solid_total",
			wrapXY,
			wrapXY,
			wrapZ);

	G4Tubs* wrap_PMT_hole = new G4Tubs("wrap_PMT_hole",
			0,
			outerRadius_pmt,
			(wrapThickness+airGapThickness)/2,
			0*deg,
			360*deg);	

//	G4ThreeVector* subVec = new G4ThreeVector(0,0,wrapX-wrapThickness/2);
	G4SubtractionSolid* wrap_solid = new G4SubtractionSolid("wrap_solid",
			wrap_solid_total,
			wrap_PMT_hole,
			rot,
			//11 mm is y-axis PMT offset
			G4ThreeVector(0,0,wrapZ-(airGapThickness+wrapThickness)/2));
//*/

/*//geometry to be used when photocathode is attached to end of Bar
	G4Box* wrap_solid = new G4Box("wrap_solid",
			wrapXY,
			wrapXY,
			wrapZ);

*/
	G4LogicalVolume* wrap_logic = new G4LogicalVolume(
			wrap_solid,
			wrapMat,
			"wrap_logic",
			0, 0, 0);
/*	
	G4PVPlacement* wrap_physic = new G4PVPlacement(
			0,
			G4ThreeVector(),
			wrap_logic,
			"wrap_physic",
			logicWorld,
			false,
			0,
			true);
*/
	
/////////////////////////////////////////////////////////////////////////////////
//                            Air Gap Geometry                                //
/////////////////////////////////////////////////////////////////////////////////	

	G4Box* airgap_solid_total = new G4Box("airgap_solid_total",
			airgapX,
			airgapY,
			airgapZ);

	G4Tubs* airgap_PMT_hole = new G4Tubs("airgap_PMT_hole",
			0,
			outerRadius_pmt,
			airGapThickness/2,
			0*deg,
			360*deg);	

	G4RotationMatrix* rot2 = new G4RotationMatrix();
//	G4ThreeVector* subVec = new G4ThreeVector(0,0,wrapX-wrapThickness/2);
	G4SubtractionSolid* airgap_solid = new G4SubtractionSolid("airgap_solid",
			airgap_solid_total,
			airgap_PMT_hole,
			rot2,
			//11 mm is y-axis PMT offset
			G4ThreeVector(0,0,airgapZ-airGapThickness/2));

	G4LogicalVolume* airgap_logic = new G4LogicalVolume(
			airgap_solid,
			worldMaterial,
			"airgap_logic",
			0, 0, 0);

	G4PVPlacement* airgap_physic = new G4PVPlacement(
			0,
			G4ThreeVector(),
			airgap_logic,
			"airgap_physic",
			wrap_logic,
			false,
			0,
			true);
	
/////////////////////////////////////////////////////////////////////////////////
//                            Scintillator Geometry                                //
/////////////////////////////////////////////////////////////////////////////////	

	G4Box* plScin_solid = new G4Box("plScin_solid",
				scintX, 
				scintY,
				scintZ);

	G4LogicalVolume* plScin_logic = new G4LogicalVolume(
				plScin_solid,
				matPlScin,
				"plScin_logic",
				0, 0, 0);

	G4PVPlacement* plScin_physic = new G4PVPlacement(
			0,
			G4ThreeVector(),
			plScin_logic,
			"plScin_physic",
			airgap_logic,
			false,
			0,
			true);



	
////////////////////////////////////Parameterization placement//////////////////////////

///*
G4Box* barStack_solid = new G4Box("barStack_solid",
			(2*wrapXY+(nBarXCount-1)*barSpacingXY)/2,
			(2*wrapXY+(nBarYCount-1)*barSpacingXY)/2,
			(wrapZ+scintZ)/2); //leaving room for the PMT one end (scintZ)

G4VPVParameterisation* barParam = new mqBarParameterisation(
			nBarXCount,
			nBarYCount, 
			(-scintZ+wrapZ)/2,
			barSpacingXY,
			wrapXY, //actually not using either of these two bottom parameters now, but we could if bars are different sizes
			wrapZ);
	
        G4double barStack_offset = (scintZ-wrapZ)/2;
	G4double layerPos = 0;
	G4double ScintSlabPlaceZ = 0;
	G4RotationMatrix rotPanL = G4RotationMatrix();
	G4RotationMatrix rotPanR = G4RotationMatrix();
	G4RotationMatrix* rotSlab2 = new G4RotationMatrix();
	G4RotationMatrix* rotSlab1 = new G4RotationMatrix();
	G4RotationMatrix* rotSlabPlace = new G4RotationMatrix();
	rotSlab2->rotateZ(180*degree);
	rotSlab1->rotateY(90*degree);
	rotSlab2->rotateY(-90*degree);
	rotPanR.rotateX(180*degree);
	rotPanR.rotateZ(80*degree);
	rotPanL.rotateY(180*degree);
	rotPanL.rotateZ(-80*degree);
	G4ThreeVector panelPlace(0,0,0);
	G4LogicalVolume* barStack_logic[nLayers];
	G4PVPlacement* barStack_physic[nLayers];
	G4PVParameterised* barParamPhys[nLayers];
        G4ThreeVector barStackPlacement(centerOffsetX,0,0);

G4Box* LeadShieldSolid = new G4Box("LeadShieldSolid",
				LeadX,
				LeadY,
				LeadZ);
		
G4LogicalVolume* LeadShieldLogic = new G4LogicalVolume(
					LeadShieldSolid,
					PbMat,
					"LeadShieldLogic",
					0,0,0);
/////////////////////////////////////////////////////////////////
G4Box* ScintSlabWrapSolidTotal = new G4Box("ScintSlabWrapSolidTotal",
				ScintSlabWrapX,
				ScintSlabWrapY,
				ScintSlabWrapZ);

G4RotationMatrix* rotSlab = new G4RotationMatrix();
	rotSlab->rotateY(90*degree);
G4SubtractionSolid* ScintSlabWrapSolid3 = new G4SubtractionSolid("ScintSlabWrapSolid3",
					ScintSlabWrapSolidTotal,
					wrap_PMT_hole,
					rotSlab,
					G4ThreeVector(
						      ScintSlabWrapX-(airGapThickness+wrapThickness)/2,-pmtOffsetY,0));	

G4SubtractionSolid* ScintSlabWrapSolid2 = new G4SubtractionSolid("ScintSlabWrapSolid2",
					ScintSlabWrapSolid3,
					wrap_PMT_hole,
					rotSlab,
					G4ThreeVector(
						      ScintSlabWrapX-(airGapThickness+wrapThickness)/2,pmtOffsetY,0));	

G4SubtractionSolid* ScintSlabWrapSolid1 = new G4SubtractionSolid("ScintSlabWrapSolid1",
					ScintSlabWrapSolid2,
					wrap_PMT_hole,
					rotSlab,
					G4ThreeVector(
						      -ScintSlabWrapX+(airGapThickness+wrapThickness)/2,pmtOffsetY,0));	

G4SubtractionSolid* ScintSlabWrapSolid = new G4SubtractionSolid("ScintSlabWrapSolid",
					ScintSlabWrapSolid1,
					wrap_PMT_hole,
					rotSlab,
					G4ThreeVector(
						      -ScintSlabWrapX+(airGapThickness+wrapThickness)/2,-pmtOffsetY,0));	

G4LogicalVolume* ScintSlabWrapLogic = new G4LogicalVolume(
					ScintSlabWrapSolid,
					wrapMat,
					"ScintSlabWrapLogic",
					0,0,0);

G4Box* ScintSlabAirGapSolidTotal = new G4Box("ScintSlabAirGapSolidTotal",
				ScintSlabAirGapX,
				ScintSlabAirGapY,
				ScintSlabAirGapZ);

G4SubtractionSolid* ScintSlabAirGapSolid3 = new G4SubtractionSolid("ScintSlabAirGapSolid3",
					ScintSlabAirGapSolidTotal,
					airgap_PMT_hole,
					rotSlab,
					G4ThreeVector(
						      ScintSlabAirGapX-airGapThickness/2,pmtOffsetY,0));	

G4SubtractionSolid* ScintSlabAirGapSolid2 = new G4SubtractionSolid("ScintSlabAirGapSolid2",
					ScintSlabAirGapSolid3,
					airgap_PMT_hole,
					rotSlab,
					G4ThreeVector(
						      ScintSlabAirGapX-airGapThickness/2,-pmtOffsetY,0));	

G4SubtractionSolid* ScintSlabAirGapSolid1 = new G4SubtractionSolid("ScintSlabAirGapSolid1",
					ScintSlabAirGapSolid2,
					airgap_PMT_hole,
					rotSlab,
					G4ThreeVector(
						      -ScintSlabAirGapX+airGapThickness/2,pmtOffsetY,0));	

G4SubtractionSolid* ScintSlabAirGapSolid = new G4SubtractionSolid("ScintSlabAirGapSolid",
					ScintSlabAirGapSolid1,
					airgap_PMT_hole,
					rotSlab,
					G4ThreeVector(
						      -ScintSlabAirGapX+airGapThickness/2,-pmtOffsetY,0));	



G4LogicalVolume* ScintSlabAirGapLogic = new G4LogicalVolume(
					ScintSlabAirGapSolid,
					worldMaterial,
					"ScintSlabAirGapLogic",
					0,0,0);
	
	G4PVPlacement* airGapSlab_physic = new G4PVPlacement(
			0,
			G4ThreeVector(),
			ScintSlabAirGapLogic,
			"airGapSlab_physic",
			ScintSlabWrapLogic,
			false,
			0,
			true);


G4Box* ScintSlabSolid = new G4Box("ScintSlabSolid",
				ScintSlabX,
				ScintSlabY,
				ScintSlabZ);

G4LogicalVolume* ScintSlabLogic = new G4LogicalVolume(
					ScintSlabSolid,
					matPlScin,
					"ScintSlabLogic",
					0,0,0);
	
	G4PVPlacement* slab_physic = new G4PVPlacement(
			0,
			G4ThreeVector(),
			ScintSlabLogic,
			"slab_physic",
			ScintSlabAirGapLogic,
			false,
			0,
			true);

//////////////////////////////////////////////////////////////////////////////
G4Box* ScintPanelWrapSolidTotal = new G4Box("ScintPanelWrapSolidTotal",
				ScintPanelWrapX,
				ScintPanelWrapY,
				ScintPanelWrapZ);


G4SubtractionSolid* ScintPanelWrapSolid = new G4SubtractionSolid("ScintPanelWrapSolid",
					ScintPanelWrapSolidTotal,
					wrap_PMT_hole,
					rotSlab,
					G4ThreeVector(0,
						      -ScintPanelWrapY+(airGapThickness+wrapThickness)/2,
						      ScintPanelZ-outerRadius_pmt));	

G4LogicalVolume* ScintPanelWrapLogic = new G4LogicalVolume(
					ScintPanelWrapSolid,
					wrapMat,
					"ScintPanelWrapLogic",
					0,0,0);


G4Box* ScintPanelAirGapSolidTotal = new G4Box("ScintPanelAirGapSolidTotal",
				ScintPanelAirGapX,
				ScintPanelAirGapY,
				ScintPanelAirGapZ);

G4SubtractionSolid* ScintPanelAirGapSolid = new G4SubtractionSolid("ScintPanelAirGapSolid",
					ScintPanelAirGapSolidTotal,
					airgap_PMT_hole,
					rotSlab,
					G4ThreeVector(0,
						      -ScintPanelAirGapY+airGapThickness/2,
						      ScintPanelZ-outerRadius_pmt));


G4LogicalVolume* ScintPanelAirGapLogic = new G4LogicalVolume(
					ScintPanelAirGapSolid,
					worldMaterial,
					"ScintPanelAirGapLogic",
					0,0,0);
	
	G4PVPlacement* airGapPanel_physic = new G4PVPlacement(
			0,
			G4ThreeVector(),
			ScintPanelAirGapLogic,
			"airGapPanel_physic",
			ScintPanelWrapLogic,
			false,
			0,
			true);


G4Box* ScintPanelSolid = new G4Box("ScintPanelSolid",
				ScintPanelX,
				ScintPanelY,
				ScintPanelZ);

G4LogicalVolume* ScintPanelLogic = new G4LogicalVolume(
					ScintPanelSolid,
					matPlScin,
					"ScintPanelLogic",
					0,0,0);

	G4PVPlacement* panel_physic = new G4PVPlacement(
			0,
			G4ThreeVector(),
			ScintPanelLogic,
			"panel_physic",
			ScintPanelAirGapLogic,
			false,
			0,
			true);

G4Box* ScintPassSlabSolid = new G4Box("ScintPassSlabSolid",
				ScintPassSlabX,
				ScintPassSlabY,
				ScintPassSlabZ);

G4LogicalVolume* ScintPassSlabLogic = new G4LogicalVolume(
					ScintPassSlabSolid,
					matPlScinNoPhoton,
					"ScintPassSlabLogic",
					0,0,0);
	

///////////////////////////////////////////////////////////////////////
G4ThreeVector slab2Placement(ScintSlabOffsetX-ScintSlabWrapX-2/2*cm,0,ScintSlabPlaceZ);
int nslabsy = 3;
int nslabsz = 4;
for(int i=0; i<nLayers; i++){
/*
	barStack_logic[i] = new G4LogicalVolume(
			barStack_solid,
			worldMaterial,
			"barStack_logic"+std::to_string(i),
			0,0,0);
	barStack_logic[i]->SetVisAttributes(visAttWorld);
*/
	
        if(i==0){ layerPos = frontLayerMid;
		  ScintSlabPlaceZ = layerPos+ScintSlabOffsetZ0;
		}

        if(i==1){ layerPos = midLayerMid;		
		  ScintSlabPlaceZ = layerPos+ScintSlabOffsetZ1;
		}

        if(i==2){ layerPos = backLayerMid;
		  ScintSlabPlaceZ = layerPos+ScintSlabOffsetZ2;
		}
        if(i==3){ layerPos = fourthLayerMid;
		  ScintSlabPlaceZ = layerPos+ScintSlabOffsetZ2;
		}

	
	barStackPlacement.setZ(layerPos+barStack_offset);	
/*		
	//place lead shielding in between layers
	if(i!=0){
		 new G4PVPlacement(
			0,
			G4ThreeVector(LeadOffsetX,0,layerPos+LeadOffsetZ),
			LeadShieldLogic,
			"LeadShieldPhys"+std::to_string(i),
			//logicWorld,
			detectorWorldLogic,
			false,
			0,
			true);
		     }
*/	

for(int z=0;z<nslabsz;z++){
	if(z==nslabsz-1) rotSlabPlace=rotSlab2;
	else rotSlabPlace=rotSlab1;
	for(int y=0;y<nslabsy;y++){
	//place scintillator slabs between layers
	new G4PVPlacement(
			rotSlabPlace, //-15, -30 worked kinda
			G4ThreeVector(ScintSlabOffsetX+pow(-1,(z+1)%2)*(ScintSlabWrapZ+0.5/2*2.54*cm)+i*(ScintSlabWrapZ*3+20*2.54*cm)-(frontLayerMid+ScintSlabOffsetZ0-ScintSlabWrapX+1*cm/2)*sin(worldRotation),centerOffsetY+(y-(double)(nslabsy-1)/2)*(2*ScintSlabWrapY+0.25*2.54*cm),(i*40*2.54*cm+(frontLayerMid+ScintSlabOffsetZ0)*cos(worldRotation)+(z-(double)(nslabsz-1)/2)*2*ScintSlabWrapX+((double)(nslabsz-1)/2-z)*4.5*2.54*cm)), //quarter inch spacing in y between slabs, 4.5in overlap. other measurements integrated also.
			ScintSlabWrapLogic,
			"ScintSlabPhys"+std::to_string(i)+std::to_string(z)+std::to_string(y),
			logicWorld,
			//detectorWorldLogic,
			false,
			18+nLayers*y+nLayers*nslabsy*z+i, //using 18 as offset out of convention. could make it 1 or something if you wanted
			true);
	}
}

/* //// passive shielding slabs ////
	new G4PVPlacement(
			0,
			G4ThreeVector(ScintSlabOffsetX+(ScintSlabWrapZ)+1/2*cm-(ScintSlabPlaceZ-ScintSlabWrapX+1*cm/2)*sin(worldRotation)+3*ScintSlabWrapZ+ScintPassSlabX+2*cm,0,(ScintSlabPlaceZ-ScintSlabWrapX+1*cm/2)*cos(worldRotation)+ScintSlabWrapX),
			ScintPassSlabLogic,
			"ScintPassSlab1"+std::to_string(i),
			logicWorld,
			false,
			100+i,
			true);
	new G4PVPlacement(
			0,
			G4ThreeVector(ScintSlabOffsetX+(ScintSlabWrapZ)+1/2*cm-(ScintSlabPlaceZ-ScintSlabWrapX+1*cm/2)*sin(worldRotation)-ScintSlabWrapZ-ScintPassSlabX-2*cm,0,(ScintSlabPlaceZ-ScintSlabWrapX+1*cm/2)*cos(worldRotation)+ScintSlabWrapX),
			ScintPassSlabLogic,
			"ScintPassSlab2"+std::to_string(i),
			logicWorld,
			false,
			104+i,
			true);
*/
/*
       //////// place three scintillator panels: one above, one on each side (rotated by 10 deg.)
	//top
	new G4PVPlacement(
			0,
			G4ThreeVector(ScintPanelOffsetXTop,0,layerPos+10*cm),
			ScintPanelWrapLogic,
			"ScintPanelTop"+std::to_string(i),
			//logicWorld,
			detectorWorldLogic,
			false,
			23+3*i,
			true);
	
	//right
			panelPlace=G4ThreeVector(ScintPanelOffsetXRight,ScintPanelOffsetYRight,layerPos+10*cm),
	new G4PVPlacement(
			G4Transform3D(rotPanR,panelPlace),
			ScintPanelWrapLogic,
			"ScintPanelRight"+std::to_string(i),
			//logicWorld,
			detectorWorldLogic,
			false,
			24+3*i,
			true);
	
	//left
			panelPlace=G4ThreeVector(ScintPanelOffsetXLeft,ScintPanelOffsetYLeft,layerPos+10*cm),
	new G4PVPlacement(
			G4Transform3D(rotPanL,panelPlace),
			ScintPanelWrapLogic,
			"ScintPanelLeft"+std::to_string(i),
			//logicWorld,
			detectorWorldLogic,
			false,
			22+3*i,
			true);
	

	barStack_physic[i] = new G4PVPlacement(
			0,
			barStackPlacement,
			barStack_logic[i],
			"barStack_physic"+std::to_string(i),
			//logicWorld,
			detectorWorldLogic,
			false,
			0,
			true);

	barParamPhys[i]= new G4PVParameterised("barParamPhys"+std::to_string(i),
			wrap_logic,
			barStack_logic[i],
			kZAxis,
			nBarXCount*nBarYCount,
			barParam,
			true); //checking overlaps
*/
	}
/*
G4ThreeVector slab2PlacementEnd(ScintSlabOffsetX-ScintSlabWrapX-2/2*cm,0,backLayerMid+ScintSlabOffsetZEnd);
	//place extra scintillator slab at the end
///*
	new G4PVPlacement(
		rotSlab1,
		G4ThreeVector(ScintSlabOffsetX+ScintSlabWrapX+2/2*cm-25*cm,0,backLayerMid+ScintSlabOffsetZEnd-53*cm),
		ScintSlabWrapLogic,
		"ScintSlabPhysEnd1",
		logicWorld,
		//detectorWorldLogic,
		false,
		21,
		true);
	new G4PVPlacement(
		rotSlab2,
		slab2PlacementEnd,
		ScintSlabWrapLogic,
		"ScintSlabPhysEnd2",
		logicWorld,
		//detectorWorldLogic,
		false,
		25,
		true);
*/
/*
	new G4PVPlacement(
		0,
		G4ThreeVector(centerOffsetX-barSpacingXY/2,centerOffsetY,fourthLayerMid),
		wrap_logic,
		"barFourthLayerLower",
		logicWorld,
		false,
		31,
		true);

	new G4PVPlacement(
		0,
		G4ThreeVector(centerOffsetX+barSpacingXY/2,centerOffsetY,fourthLayerMid),
		wrap_logic,
		"barFourthLayerUpper",
		logicWorld,
		false,
		32,
		true);
 */
//==========================================================================
    //---------------PMT Geometry---------------
    //==========================================================================
	 
          //---------------------------------------------------
          //Borosilicate glass (Schott BK7) (8" PMT window);
          //---------------------------------------------------
          G4Material* matBorGlass = new G4Material("BorGlass", 2.51 * g / cm3, 6);
          matBorGlass->AddElement(elB, 0.040064);
          matBorGlass->AddElement(elO, 0.539562);
          matBorGlass->AddElement(elNa, 0.028191);
          matBorGlass->AddElement(elAl, 0.011644);
          matBorGlass->AddElement(elSi, 0.377220);
          matBorGlass->AddElement(elK, 0.003321);

          //BiAlkali photocathode (NIM A567, p.222) K2CsSb;
          //For the references for the optical properties see ../ref/ComplexRefractionSpectrum_KCSSb.pdf
          //---------------------------------------------------
          // exact composition of the bialkali is unknown, density is the 'realistic' guess!;
          G4Material* matBiAlkali = new G4Material("matBiAlkali", 1.3 * g / cm3, 3);
          matBiAlkali->AddElement(elK, 2);
          matBiAlkali->AddElement(elCs, 1);
          matBiAlkali->AddElement(elSb, 1);

//////////////////// defining phCathSolid as PMT so we can parameterise it /////////////
///*
	 G4Tubs* phCathSolid = new G4Tubs(
			 "photocath_tube",
			 0,
			 //outerRadius_pmt,
			 outerRadius_cath,
	//		 (airGapThickness+wrapThickness)/2,
			 height_pmt/2,
			 0*deg,
			 360*deg);

	 G4LogicalVolume* phCathLog = new G4LogicalVolume(phCathSolid,
					matBiAlkali,
					"phCathLog");

//	 G4ThreeVector phCathPosition(0,0,(-height_pmt/2 + height_cath/2));
//	 G4ThreeVector phCathPosition(0,0,scintZ+(airGapThickness+wrapThickness)/2);
/*
	 G4PVPlacement* phCathPhys = new G4PVPlacement(
			 0,
			 phCathPosition,
			 phCathLog,
			 "phCathPhys",
			 wrap_logic,
//			 pmtLog,
			 false,
			 0,
			 true);
*/
////////// add pmt as daughter of photocathode (impacts visualization only)

	 G4Tubs* pmtSolid = new G4Tubs(
			 "pmt_tube",
			 //0,
			 outerRadius_cath,
			 outerRadius_pmt,
		         height_pmt/2,
		         //(height_pmt-height_cath)/2,
		         0*deg,
		         360*deg);

	 G4LogicalVolume* pmtLog = new G4LogicalVolume(
			 pmtSolid,
	 	 	 //matBorGlass, //actually made of matBorGlass, but the material isn't a significant source of bkgd
			 worldMaterial, //so, going with air (world material) instead https://www.chem.uci.edu/~unicorn/243/handouts/pmt.pdf
 	 		"pmtLog");
/*
	G4PVPlacement* pmtPhys = new G4PVPlacement(
			 0,
			 G4ThreeVector(0,0,0),
			 //G4ThreeVector(0,0,(height_cath+50*mm)/2),
			 //G4ThreeVector(0,0,(height_cath+0.1*mm)/2),
			 pmtLog,
			 "pmtPhys",
			 phCathLog,
			 false,
			 0,
			 true);
*/


//mu metal shielding
G4Box* muMetalOuterShield = new G4Box("muMetalOuterShield",
			(2*outerRadius_pmt+(nBarXCount-1)*barSpacingXY)/2+muMetalThickness,
			(2*outerRadius_pmt+(nBarXCount-1)*barSpacingXY)/2+muMetalThickness,
			//(2*wrapXY+(nBarXCount-1)*barSpacingXY)/2+muMetalThickness,
			//(2*wrapXY+(nBarXCount-1)*barSpacingXY)/2+muMetalThickness,
			height_pmt/2+muMetalThickness/2);

////////////////////////////// PMT Parameterization placement ////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////
///*

G4Box* PMTStack_solid = new G4Box("PMTStack_solid",
			(2*outerRadius_pmt+(nBarXCount-1)*barSpacingXY)/2,
			(2*outerRadius_pmt+(nBarXCount-1)*barSpacingXY)/2,
			//(2*wrapXY+(nBarXCount-1)*barSpacingXY)/2,
			//(2*wrapXY+(nBarXCount-1)*barSpacingXY)/2,
			height_pmt/2);

G4SubtractionSolid* muMetalShield = new G4SubtractionSolid("muMetalShield",
					muMetalOuterShield,
					PMTStack_solid,
					rot,
					G4ThreeVector(0,0,-muMetalThickness/2));

G4LogicalVolume* muMetalShield_logic = new G4LogicalVolume(
					muMetalShield,
					worldMaterial,
					"muMetalShield_logic",
					0,0,0);			

        G4double PMTStack_offset = scintZ+height_pmt/2;

G4VPVParameterisation* PMTParam = new mqPMTParameterisation(
			nBarXCount,
			nBarYCount,
			0,
			barSpacingXY,
			outerRadius_pmt, //actually not using either of these two bottom parameters now, but we could if PMTs are different sizes
			height_pmt/2);

	G4LogicalVolume* PMTStack_logic[nLayers];
	G4PVPlacement* PMTStack_physic[nLayers];
	//G4PVPlacement* muMetalShield_physic[nLayers];
	G4PVParameterised* PMTParamPhys[nLayers];

	G4double PanelToPMT = ScintPanelY+height_pmt/2;
	G4double LPanelToPMTX= -PanelToPMT*cos(10*degree);
	G4double RPanelToPMTX= -PanelToPMT*cos(10*degree);
	G4double LPanelToPMTY= -PanelToPMT*sin(10*degree);
	G4double RPanelToPMTY= PanelToPMT*sin(10*degree);

	G4ThreeVector muMetalPlacement(centerOffsetX, 0, muMetalThickness/2); //adjusting Z in loop
        G4ThreeVector PMTStackPlacement(centerOffsetX, 0, 0); //adjusting Z in loop
//	G4ThreeVector PMTSlabPlacement(ScintSlabOffsetX+ScintSlabWrapX+2/2*cm, 0, 0); //adjusting Z in loop
//	G4ThreeVector PMTSlabPlacement2(ScintSlabOffsetX-ScintSlabWrapX-2/2*cm, 0, 0); //adjusting Z in loop
	G4ThreeVector PMTSlabPlacement(-(ScintSlabOffsetX+(ScintSlabWrapZ)+1/2*cm)*sin(worldRotation), 0, 0); //adjusting Z in loop
	G4ThreeVector PMTSlabPlacement2(-(ScintSlabOffsetX-(ScintSlabWrapZ)-1/2*cm)*sin(worldRotation), 0,0); //adjusting Z in loop
	G4ThreeVector PMTSlabPlacement3(-(ScintSlabOffsetX+(ScintSlabWrapZ)+1/2*cm)*sin(worldRotation), 0, 0); //adjusting Z in loop
	G4ThreeVector PMTSlabPlacement4(-(ScintSlabOffsetX-(ScintSlabWrapZ)-1/2*cm)*sin(worldRotation), 0,0); //adjusting Z in loop

	G4ThreeVector PMTPanelPlacementTop(ScintPanelOffsetXTop, -ScintPanelY-height_pmt/2, 0); //adjusting Z in loop
	G4ThreeVector PMTPanelPlacementRight(ScintPanelOffsetXRight+RPanelToPMTX, ScintPanelOffsetYRight+RPanelToPMTY, -ScintPanelZ+outerRadius_pmt); //adjusting Z in loop
	G4ThreeVector PMTPanelPlacementLeft(ScintPanelOffsetXLeft+LPanelToPMTX, ScintPanelOffsetYLeft+LPanelToPMTY, -ScintPanelZ+outerRadius_pmt); //adjusting Z in loop
	
	G4RotationMatrix* pmtRotSlab = new G4RotationMatrix();
	G4RotationMatrix* pmtRotSlab2 = new G4RotationMatrix();
	G4RotationMatrix* pmtRotSlabPlace = new G4RotationMatrix();
	G4RotationMatrix* pmtRotPanelTop = new G4RotationMatrix();
	G4RotationMatrix* pmtRotPanelLeft = new G4RotationMatrix();
	G4RotationMatrix* pmtRotPanelRight = new G4RotationMatrix();

	//pmtRotSlab->rotateX(90*degree);
	//pmtRotSlab2->rotateX(-90*degree);
	//pmtRotSlab->rotateY(90*degree-worldRotation);
	//pmtRotSlab2->rotateY(-90*degree+worldRotation);
//	pmtRotSlab->rotateZ(-180*degree);
	pmtRotSlab->rotateY(180*degree);
	pmtRotPanelTop->rotateX(-90*degree);
	
	pmtRotPanelRight->rotateX(90*degree);
	pmtRotPanelRight->rotateY(80*degree);
	
	pmtRotPanelLeft->rotateX(-90*degree);
	pmtRotPanelLeft->rotateY(80*degree);
		
	G4double pmtTotalOffsetZ = 0;
	G4double pmtOffsetZ=0;
	G4double pmtOffsetZ1=0;
	G4double pmtOffsetZ2=0;
for(int j=0; j<nLayers; j++){
/*
	PMTStack_logic[j] = new G4LogicalVolume(
			PMTStack_solid,
			worldMaterial,
			"PMTStack_logic"+std::to_string(j),
			0,0,0);
	//PMTStack_logic[j]->SetVisAttributes(visAttribplScin);
	PMTStack_logic[j]->SetVisAttributes(visAttWorld);
*/

        if(j==0){ layerPos = frontLayerMid;
		  ScintSlabPlaceZ = layerPos+ScintSlabOffsetZ0;
        	   pmtTotalOffsetZ=frontLayerMid+PMTStack_offset;
		}

        if(j==1){ layerPos = midLayerMid;		
		  ScintSlabPlaceZ = layerPos+ScintSlabOffsetZ1;
        	  pmtTotalOffsetZ=midLayerMid+PMTStack_offset;
		}

        if(j==2){ layerPos = backLayerMid;
		  ScintSlabPlaceZ = layerPos+ScintSlabOffsetZ2;
        	  pmtTotalOffsetZ=backLayerMid+PMTStack_offset;
		}
        if(j==3){ layerPos = fourthLayerMid;
		  ScintSlabPlaceZ = layerPos+ScintSlabOffsetZ2;
        	  pmtTotalOffsetZ=fourthLayerMid+PMTStack_offset;
		}
	pmtOffsetZ1=-(ScintSlabX+height_pmt/2);
	pmtOffsetZ2=ScintSlabX+height_pmt/2;
for(int z=0;z<nslabsz;z++){
//        if(z==nslabsz-1) {pmtRotSlabPlace=pmtRotSlab2; pmtOffsetZ=ScintSlabX+height_pmt/2;}
  //      else {pmtRotSlabPlace=pmtRotSlab; pmtOffsetZ=-(ScintSlabX+height_pmt/2);}
        for(int y=0;y<nslabsy;y++){

	//PMTs attached to slabs
	new G4PVPlacement(
			pmtRotSlab, //pmtRotSlabPlace
                        //G4ThreeVector(ScintSlabOffsetX+pow(-1,(z+1)%2)*(ScintSlabWrapZ+0.5/2*2.54*cm)-(ScintSlabPlaceZ-ScintSlabWrapX+1*cm/2)*sin(worldRotation),pmtOffsetY+centerOffsetY+(y-(double)(nslabsy-1)/2)*(2*ScintSlabWrapY+0.25*2.54*cm),(ScintSlabPlaceZ*cos(worldRotation)+(z-(double)(nslabsz-1)/2)*2*ScintSlabWrapX+((double)(nslabsz-1)/2-z)*1*cm)+pmtOffsetZ1),
			G4ThreeVector(ScintSlabOffsetX+pow(-1,(z+1)%2)*(ScintSlabWrapZ+0.5/2*2.54*cm)+j*(ScintSlabWrapZ*3+20*2.54*cm)-(frontLayerMid+ScintSlabOffsetZ0-ScintSlabWrapX+1*cm/2)*sin(worldRotation),pmtOffsetY+centerOffsetY+(y-(double)(nslabsy-1)/2)*(2*ScintSlabWrapY+0.25*2.54*cm),(j*40*2.54*cm+(frontLayerMid+ScintSlabOffsetZ0)*cos(worldRotation)+(z-(double)(nslabsz-1)/2)*2*ScintSlabWrapX+((double)(nslabsz-1)/2-z)*4.5*2.54*cm)+pmtOffsetZ1), //quarter inch spacing in y between slabs, 4.5in overlap. other measurements integrated also.
			phCathLog,
		        "ScintSlabPMT"+std::to_string(j)+std::to_string(z)+std::to_string(y),
      		        logicWorld,
			//detectorWorldLogic,
		        false,
		        18+4*(nLayers*y+nLayers*nslabsy*z+j), //setting copy numbers for the slabs to be 18,19,20,21,22,23,24,25
	                true);
	new G4PVPlacement(
			pmtRotSlab, //pmtRotSlabPlace
			G4ThreeVector(ScintSlabOffsetX+pow(-1,(z+1)%2)*(ScintSlabWrapZ+0.5/2*2.54*cm)+j*(ScintSlabWrapZ*3+20*2.54*cm)-(frontLayerMid+ScintSlabOffsetZ0-ScintSlabWrapX+1*cm/2)*sin(worldRotation),pmtOffsetY+centerOffsetY+(y-(double)(nslabsy-1)/2)*(2*ScintSlabWrapY+0.25*2.54*cm),(j*40*2.54*cm+(frontLayerMid+ScintSlabOffsetZ0)*cos(worldRotation)+(z-(double)(nslabsz-1)/2)*2*ScintSlabWrapX+((double)(nslabsz-1)/2-z)*4.5*2.54*cm)+pmtOffsetZ1), //quarter inch spacing in y between slabs, 4.5in overlap. other measurements integrated also.
                        //G4ThreeVector(ScintSlabOffsetX+pow(-1,(z+1)%2)*(ScintSlabWrapZ+0.5/2*2.54*cm)-(ScintSlabPlaceZ-ScintSlabWrapX+1*cm/2)*sin(worldRotation),pmtOffsetY+centerOffsetY+(y-(double)(nslabsy-1)/2)*(2*ScintSlabWrapY+0.25*2.54*cm),(ScintSlabPlaceZ*cos(worldRotation)+(z-(double)(nslabsz-1)/2)*2*ScintSlabWrapX+((double)(nslabsz-1)/2-z)*1*cm)+pmtOffsetZ1),
			pmtLog,
		        "ScintSlabPMT"+std::to_string(j)+std::to_string(z)+std::to_string(y),
      		        logicWorld,
			//detectorWorldLogic,
		        false,
		        18+4*(nLayers*y+nLayers*nslabsy*z+j), //setting copy numbers for the slabs to be 18,19,20,21,22,23,24,25
	                true);
	
	new G4PVPlacement(
			pmtRotSlab, //pmtRotSlabPlace
			G4ThreeVector(ScintSlabOffsetX+pow(-1,(z+1)%2)*(ScintSlabWrapZ+0.5/2*2.54*cm)+j*(ScintSlabWrapZ*3+20*2.54*cm)-(frontLayerMid+ScintSlabOffsetZ0-ScintSlabWrapX+1*cm/2)*sin(worldRotation),-pmtOffsetY+centerOffsetY+(y-(double)(nslabsy-1)/2)*(2*ScintSlabWrapY+0.25*2.54*cm),(j*40*2.54*cm+(frontLayerMid+ScintSlabOffsetZ0)*cos(worldRotation)+(z-(double)(nslabsz-1)/2)*2*ScintSlabWrapX+((double)(nslabsz-1)/2-z)*4.5*2.54*cm)+pmtOffsetZ1), //quarter inch spacing in y between slabs, 4.5in overlap. other measurements integrated also.
                        //G4ThreeVector(ScintSlabOffsetX+pow(-1,(z+1)%2)*(ScintSlabWrapZ+0.5/2*2.54*cm)-(ScintSlabPlaceZ-ScintSlabWrapX+1*cm/2)*sin(worldRotation),-pmtOffsetY+centerOffsetY+(y-(double)(nslabsy-1)/2)*(2*ScintSlabWrapY+0.25*2.54*cm),(ScintSlabPlaceZ*cos(worldRotation)+(z-(double)(nslabsz-1)/2)*2*ScintSlabWrapX+((double)(nslabsz-1)/2-z)*1*cm)+pmtOffsetZ1),
			phCathLog,
		        "ScintSlabPMT"+std::to_string(j)+std::to_string(z)+std::to_string(y),
      		        logicWorld,
			//detectorWorldLogic,
		        false,
		        19+4*(nLayers*y+nLayers*nslabsy*z+j), //setting copy numbers for the slabs to be 18,19,20,21,22,23,24,25
	                true);
	new G4PVPlacement(
			pmtRotSlab, //pmtRotSlabPlace
			G4ThreeVector(ScintSlabOffsetX+pow(-1,(z+1)%2)*(ScintSlabWrapZ+0.5/2*2.54*cm)+j*(ScintSlabWrapZ*3+20*2.54*cm)-(frontLayerMid+ScintSlabOffsetZ0-ScintSlabWrapX+1*cm/2)*sin(worldRotation),-pmtOffsetY+centerOffsetY+(y-(double)(nslabsy-1)/2)*(2*ScintSlabWrapY+0.25*2.54*cm),(j*40*2.54*cm+(frontLayerMid+ScintSlabOffsetZ0)*cos(worldRotation)+(z-(double)(nslabsz-1)/2)*2*ScintSlabWrapX+((double)(nslabsz-1)/2-z)*4.5*2.54*cm)+pmtOffsetZ1), //quarter inch spacing in y between slabs, 4.5in overlap. other measurements integrated also.
                        //G4ThreeVector(ScintSlabOffsetX+pow(-1,(z+1)%2)*(ScintSlabWrapZ+0.5/2*2.54*cm)-(ScintSlabPlaceZ-ScintSlabWrapX+1*cm/2)*sin(worldRotation),-pmtOffsetY+centerOffsetY+(y-(double)(nslabsy-1)/2)*(2*ScintSlabWrapY+0.25*2.54*cm),(ScintSlabPlaceZ*cos(worldRotation)+(z-(double)(nslabsz-1)/2)*2*ScintSlabWrapX+((double)(nslabsz-1)/2-z)*1*cm)+pmtOffsetZ1),
			pmtLog,
		        "ScintSlabPMT"+std::to_string(j)+std::to_string(z)+std::to_string(y),
      		        logicWorld,
			//detectorWorldLogic,
		        false,
		        19+4*(nLayers*y+nLayers*nslabsy*z+j), //setting copy numbers for the slabs to be 18,19,20,21,22,23,24,25
	                true);
  
	new G4PVPlacement(
			pmtRotSlab2,
			G4ThreeVector(ScintSlabOffsetX+pow(-1,(z+1)%2)*(ScintSlabWrapZ+0.5/2*2.54*cm)+j*(ScintSlabWrapZ*3+20*2.54*cm)-(frontLayerMid+ScintSlabOffsetZ0-ScintSlabWrapX+1*cm/2)*sin(worldRotation),-pmtOffsetY+centerOffsetY+(y-(double)(nslabsy-1)/2)*(2*ScintSlabWrapY+0.25*2.54*cm),(j*40*2.54*cm+(frontLayerMid+ScintSlabOffsetZ0)*cos(worldRotation)+(z-(double)(nslabsz-1)/2)*2*ScintSlabWrapX+((double)(nslabsz-1)/2-z)*4.5*2.54*cm)+pmtOffsetZ2), //quarter inch spacing in y between slabs, 4.5in overlap. other measurements integrated also.
                        //G4ThreeVector(ScintSlabOffsetX+pow(-1,(z+1)%2)*(ScintSlabWrapZ+0.5/2*2.54*cm)-(ScintSlabPlaceZ-ScintSlabWrapX+1*cm/2)*sin(worldRotation),-pmtOffsetY+centerOffsetY+(y-(double)(nslabsy-1)/2)*(2*ScintSlabWrapY+0.25*2.54*cm),(ScintSlabPlaceZ*cos(worldRotation)+(z-(double)(nslabsz-1)/2)*2*ScintSlabWrapX+((double)(nslabsz-1)/2-z)*1*cm)+pmtOffsetZ2),
			phCathLog,
		        "ScintSlabPMT"+std::to_string(j)+std::to_string(z)+std::to_string(y),
      		        logicWorld,
			//detectorWorldLogic,
		        false,
		        20+4*(nLayers*y+nLayers*nslabsy*z+j), //setting copy numbers for the slabs to be 18,19,20,21,22,23,24,25
	                true);
	new G4PVPlacement(
			pmtRotSlab2,
			G4ThreeVector(ScintSlabOffsetX+pow(-1,(z+1)%2)*(ScintSlabWrapZ+0.5/2*2.54*cm)+j*(ScintSlabWrapZ*3+20*2.54*cm)-(frontLayerMid+ScintSlabOffsetZ0-ScintSlabWrapX+1*cm/2)*sin(worldRotation),-pmtOffsetY+centerOffsetY+(y-(double)(nslabsy-1)/2)*(2*ScintSlabWrapY+0.25*2.54*cm),(j*40*2.54*cm+(frontLayerMid+ScintSlabOffsetZ0)*cos(worldRotation)+(z-(double)(nslabsz-1)/2)*2*ScintSlabWrapX+((double)(nslabsz-1)/2-z)*4.5*2.54*cm)+pmtOffsetZ2), //quarter inch spacing in y between slabs, 4.5in overlap. other measurements integrated also.
                        //G4ThreeVector(ScintSlabOffsetX+pow(-1,(z+1)%2)*(ScintSlabWrapZ+0.5/2*2.54*cm)-(ScintSlabPlaceZ-ScintSlabWrapX+1*cm/2)*sin(worldRotation),-pmtOffsetY+centerOffsetY+(y-(double)(nslabsy-1)/2)*(2*ScintSlabWrapY+0.25*2.54*cm),(ScintSlabPlaceZ*cos(worldRotation)+(z-(double)(nslabsz-1)/2)*2*ScintSlabWrapX+((double)(nslabsz-1)/2-z)*1*cm)+pmtOffsetZ2),
			pmtLog,
		        "ScintSlabPMT"+std::to_string(j)+std::to_string(z)+std::to_string(y),
      		        logicWorld,
			//detectorWorldLogic,
		        false,
		        20+4*(nLayers*y+nLayers*nslabsy*z+j), //setting copy numbers for the slabs to be 18,19,20,21,22,23,24,25
	                true);
  
	new G4PVPlacement(
			pmtRotSlab2,
			G4ThreeVector(ScintSlabOffsetX+pow(-1,(z+1)%2)*(ScintSlabWrapZ+0.5/2*2.54*cm)+j*(ScintSlabWrapZ*3+20*2.54*cm)-(frontLayerMid+ScintSlabOffsetZ0-ScintSlabWrapX+1*cm/2)*sin(worldRotation),pmtOffsetY+centerOffsetY+(y-(double)(nslabsy-1)/2)*(2*ScintSlabWrapY+0.25*2.54*cm),(j*40*2.54*cm+(frontLayerMid+ScintSlabOffsetZ0)*cos(worldRotation)+(z-(double)(nslabsz-1)/2)*2*ScintSlabWrapX+((double)(nslabsz-1)/2-z)*4.5*2.54*cm)+pmtOffsetZ2), //quarter inch spacing in y between slabs, 4.5in overlap. other measurements integrated also.
                        //G4ThreeVector(ScintSlabOffsetX+pow(-1,(z+1)%2)*(ScintSlabWrapZ+0.5/2*2.54*cm)-(ScintSlabPlaceZ-ScintSlabWrapX+1*cm/2)*sin(worldRotation),pmtOffsetY+centerOffsetY+(y-(double)(nslabsy-1)/2)*(2*ScintSlabWrapY+0.25*2.54*cm),(ScintSlabPlaceZ*cos(worldRotation)+(z-(double)(nslabsz-1)/2)*2*ScintSlabWrapX+((double)(nslabsz-1)/2-z)*1*cm)+pmtOffsetZ2),
			phCathLog,
		        "ScintSlabPMT"+std::to_string(j)+std::to_string(z)+std::to_string(y),
      		        logicWorld,
			//detectorWorldLogic,
		        false,
		        21+4*(nLayers*y+nLayers*nslabsy*z+j), //setting copy numbers for the slabs to be 18,19,20,21,22,23,24,25
	                true);
	new G4PVPlacement(
			pmtRotSlab2,
			G4ThreeVector(ScintSlabOffsetX+pow(-1,(z+1)%2)*(ScintSlabWrapZ+0.5/2*2.54*cm)+j*(ScintSlabWrapZ*3+20*2.54*cm)-(frontLayerMid+ScintSlabOffsetZ0-ScintSlabWrapX+1*cm/2)*sin(worldRotation),pmtOffsetY+centerOffsetY+(y-(double)(nslabsy-1)/2)*(2*ScintSlabWrapY+0.25*2.54*cm),(j*40*2.54*cm+(frontLayerMid+ScintSlabOffsetZ0)*cos(worldRotation)+(z-(double)(nslabsz-1)/2)*2*ScintSlabWrapX+((double)(nslabsz-1)/2-z)*4.5*2.54*cm)+pmtOffsetZ2), //quarter inch spacing in y between slabs, 4.5in overlap. other measurements integrated also.
                        //G4ThreeVector(ScintSlabOffsetX+pow(-1,(z+1)%2)*(ScintSlabWrapZ+0.5/2*2.54*cm)-(ScintSlabPlaceZ-ScintSlabWrapX+1*cm/2)*sin(worldRotation),pmtOffsetY+centerOffsetY+(y-(double)(nslabsy-1)/2)*(2*ScintSlabWrapY+0.25*2.54*cm),(ScintSlabPlaceZ*cos(worldRotation)+(z-(double)(nslabsz-1)/2)*2*ScintSlabWrapX+((double)(nslabsz-1)/2-z)*1*cm)+pmtOffsetZ2),
			pmtLog,
		        "ScintSlabPMT"+std::to_string(j)+std::to_string(z)+std::to_string(y),
      		        logicWorld,
			//detectorWorldLogic,
		        false,
		        21+4*(nLayers*y+nLayers*nslabsy*z+j), //setting copy numbers for the slabs to be 18,19,20,21,22,23,24,25
	                true);
	}
}
/*
	PMTSlabPlacement.setX(ScintSlabOffsetX+(ScintSlabWrapZ)+1/2*cm-(ScintSlabPlaceZ-ScintSlabWrapX+1*cm/2)*sin(worldRotation));
       	PMTSlabPlacement2.setX(ScintSlabOffsetX-(ScintSlabWrapZ)-1/2*cm-(ScintSlabPlaceZ+ScintSlabWrapX-1*cm/2)*sin(worldRotation)-45.5*cm);
	PMTSlabPlacement3.setX(ScintSlabOffsetX+(ScintSlabWrapZ)+1/2*cm-(ScintSlabPlaceZ-ScintSlabWrapX+1*cm/2)*sin(worldRotation));
       	PMTSlabPlacement4.setX(ScintSlabOffsetX-(ScintSlabWrapZ)-1/2*cm-(ScintSlabPlaceZ+ScintSlabWrapX-1*cm/2)*sin(worldRotation)-45.5*cm);
       	PMTSlabPlacement.setZ((ScintSlabPlaceZ-ScintSlabWrapX+1*cm/2)*cos(worldRotation)-(ScintSlabX+height_pmt/2));
       	PMTSlabPlacement2.setZ((ScintSlabPlaceZ+ScintSlabWrapX-1*cm/2)*cos(worldRotation)+(ScintSlabX+height_pmt/2)+22*cm);
       	PMTSlabPlacement3.setZ((ScintSlabPlaceZ-ScintSlabWrapX+1*cm/2)*cos(worldRotation)-(ScintSlabX+height_pmt/2));
       	PMTSlabPlacement4.setZ((ScintSlabPlaceZ+ScintSlabWrapX-1*cm/2)*cos(worldRotation)+(ScintSlabX+height_pmt/2)+22*cm);
       	PMTPanelPlacementTop.setZ(layerPos+ScintPanelZ-outerRadius_pmt+10*cm);
       	PMTPanelPlacementLeft.setZ(layerPos-ScintPanelZ+outerRadius_pmt+10*cm);
      	PMTPanelPlacementRight.setZ(layerPos-ScintPanelZ+outerRadius_pmt+10*cm);
	muMetalPlacement.setZ(pmtTotalOffsetZ+muMetalThickness/2);
	
	PMTSlabPlacement.setY(-ScintSlabWrapY-1/2*cm);
	PMTSlabPlacement2.setY(-ScintSlabWrapY-1/2*cm);
	PMTSlabPlacement3.setY(ScintSlabWrapY+1/2*cm);
	PMTSlabPlacement4.setY(ScintSlabWrapY+1/2*cm);
*/
/*	//PMTs attached to slabs
	new G4PVPlacement(
			pmtRotSlab,
			PMTSlabPlacement,
			phCathLog,
		        "ScintSlabPMT1"+std::to_string(j),
      		        logicWorld,
			//detectorWorldLogic,
		        false,
		        18+j, //setting copy numbers for the slabs to be 18,19,20,21
	                true);
	//PMTs attached to slabs
	new G4PVPlacement(
			pmtRotSlab2,
			PMTSlabPlacement2,
			phCathLog,
		        "ScintSlabPMT2"+std::to_string(j),
      		        logicWorld,
			//detectorWorldLogic,
		        false,
		        22+j, //setting copy numbers for the slabs to be 18,19,20,21
	                true);
	//PMTs attached to slabs
	new G4PVPlacement(
			pmtRotSlab,
			PMTSlabPlacement3,
			phCathLog,
		        "ScintSlabPMT3"+std::to_string(j),
      		        logicWorld,
			//detectorWorldLogic,
		        false,
		        26+j, //setting copy numbers for the slabs to be 18,19,20,21
	                true);
	//PMTs attached to slabs
	new G4PVPlacement(
			pmtRotSlab2,
			PMTSlabPlacement4,
			phCathLog,
		        "ScintSlabPMT4"+std::to_string(j),
      		        logicWorld,
			//detectorWorldLogic,
		        false,
		        30+j, //setting copy numbers for the slabs to be 18,19,20,21
	                true);
*/	
	//PMTs attached to upper and side scintillator panels
/*
	//left panel
	new G4PVPlacement(
			pmtRotPanelLeft,
			PMTPanelPlacementLeft,
			phCathLog,
			"ScintPanelPMTLeft"+std::to_string(j),
			detectorWorldLogic,
			//logicWorld,
			false,
			22+3*j,
			true);
	//top panel
	new G4PVPlacement(
			pmtRotPanelTop,
			PMTPanelPlacementTop,
			phCathLog,
			"ScintPanelPMTTop"+std::to_string(j),
			//logicWorld,
			detectorWorldLogic,
			false,
			23+3*j,
			true);

	//right panel
	new G4PVPlacement(
			pmtRotPanelRight,
			PMTPanelPlacementRight,
			phCathLog,
			"ScintPanelPMTRight"+std::to_string(j),
			//logicWorld,
			detectorWorldLogic,
			false,
			24+3*j,
			true);
*/
}
/*
       	PMTSlabPlacement.setZ(backLayerMid+ScintSlabOffsetZEnd-(ScintSlabX+height_pmt/2)*cos(worldRotation)-53*cm);
       	PMTSlabPlacement2.setZ(backLayerMid+ScintSlabOffsetZEnd+(ScintSlabX+height_pmt/2)*cos(worldRotation));
	//PMTs attached to slab at end of detector
	new G4PVPlacement(
			pmtRotSlab,
			PMTSlabPlacement,
			phCathLog,
		        "ScintSlabPMTEnd1",
      		        logicWorld,
			//detectorWorldLogic,
		        false,
		        21, //setting copy numbers for the slabs to be 18,19,20,21
	                true);
	new G4PVPlacement(
			pmtRotSlab2,
			PMTSlabPlacement2,
			phCathLog,
		        "ScintSlabPMTEnd2",
      		        logicWorld,
			//detectorWorldLogic,
		        false,
		        25, //setting copy numbers for the slabs to be 18,19,20,21
	                true);
*/
/*
	new G4PVPlacement(
		0,
		G4ThreeVector(centerOffsetX-barSpacingXY/2,centerOffsetY,fourthLayerMid+PMTStack_offset),
		phCathLog,
		"PMTFourthLayerLower",
		logicWorld,
		false,
		31,
		true);
 
	new G4PVPlacement(
		0,
		G4ThreeVector(centerOffsetX+barSpacingXY/2,centerOffsetY,fourthLayerMid+PMTStack_offset),
		phCathLog,
		"PMTFourthLayerUpper",
		logicWorld,
		false,
		32,
		true);
*/
////////////////    G4 Vis attributes        /////////////////////////////
	
	G4VisAttributes* visAttribplScin = new G4VisAttributes(G4Colour::Cyan());
	visAttribplScin->SetVisibility(true);

	G4VisAttributes* visAttribplScinExt = new G4VisAttributes(G4Colour::Cyan());
	visAttribplScinExt->SetVisibility(false);
	
	G4VisAttributes* visAttribWrap = new G4VisAttributes(
			G4Colour::White());
	visAttribWrap->SetVisibility(true);

	G4VisAttributes* visAttribWrapExt = new G4VisAttributes(
			G4Colour::White());
	visAttribWrapExt->SetColour(0.5,0.5,0.5,0.3);
	visAttribWrapExt->SetVisibility(true);

	G4VisAttributes* visAttribLead = new G4VisAttributes(
			G4Colour::Black());
	visAttribLead->SetVisibility(true);
	
	G4VisAttributes* visAttribMuMetal = new G4VisAttributes(
			G4Colour::Black());
	visAttribMuMetal->SetColour(0,0,0,0.1);
	visAttribMuMetal->SetVisibility(true);
	

	wrap_logic->SetVisAttributes(visAttribWrap);
	//ScintPanelWrapLogic->SetVisAttributes(visAttribWrapExt);
	ScintSlabWrapLogic->SetVisAttributes(visAttribWrap);
	//ScintPanelAirGapLogic->SetVisAttributes(visAttribplScinExt);
	ScintSlabAirGapLogic->SetVisAttributes(visAttribplScinExt);
	plScin_logic->SetVisAttributes(visAttribplScin);
	ScintSlabLogic->SetVisAttributes(visAttribplScin);
	//ScintPanelLogic->SetVisAttributes(visAttribplScinExt);
	ScintPassSlabLogic->SetVisAttributes(visAttribWrap);
	
	LeadShieldLogic->SetVisAttributes(visAttribLead);
	muMetalShield_logic->SetVisAttributes(visAttribMuMetal);
	//detectorWorldLogic->SetVisAttributes(visAttribWrapExt); //AttWorld

////////////////////////////////////////////////////////////////////////////
// Long Al Support Placement
if(SupportStructure){
/*
        G4Box* AlSupportSolidFull = new G4Box("AlSupportSolidFull",
                                                AlSupportX,
                                                AlSupportY,
                                                AlSupportZ);

        G4Box* AlSupportSolidHollow = new G4Box("AlSupportSolidHollow",
                                                AlSupportX-AlThickness,
                                                AlSupportY-AlThickness,
                                                AlSupportZ-AlThickness);

	G4SubtractionSolid* AlSupportSolid = new G4SubtractionSolid("AlSupportSolid",
						AlSupportSolidFull,
						AlSupportSolidHollow,
						rot, //unrotated
						G4ThreeVector(0,0,0));

        G4LogicalVolume* AlSupportLogic = new G4LogicalVolume(
                                                AlSupportSolid,
                                                AlMat,
                                                //steelMat,
                                                "AlSupportLogic",
                                                0,0,0);

        G4PVPlacement* AlSupportPhys = new G4PVPlacement(
                                        0,
                                        G4ThreeVector(AlSupportXOffset,0,0),
                                        AlSupportLogic,
                                        "AlSupportPhys",
                                        //logicWorld,
					detectorWorldLogic,
                                        false,
                                        0,
                                        true);
	G4VisAttributes* visAttribAl = new G4VisAttributes(
			G4Colour::Gray());
	visAttribAl->SetVisibility(true);	
	AlSupportLogic->SetVisAttributes(visAttribAl);
*/
// add in box nearby, made of steel, representing the support structure
/*
	G4Box* SteelSupportBoxFull = new G4Box("SteelSupportBoxFull",
					SteelSideLength,
					SteelSideLength,
					SteelSideLength);

	G4Box* SteelSupportBoxHollow = new G4Box("SteelSupportBoxHollow",
					SteelSideLength-SteelThickness,
					SteelSideLength-SteelThickness,
					SteelSideLength-SteelThickness);
	
	G4SubtractionSolid* SteelSupportBox = new G4SubtractionSolid("SteelSupportBox",
					SteelSupportBoxFull,
					SteelSupportBoxHollow,
					rot,
					G4ThreeVector(0,0,0));

	G4LogicalVolume* SteelSupportLogic = new G4LogicalVolume(
					SteelSupportBox,
					steelMat,
					"SteelSupportLogic",
					0,0,0);

	G4PVPlacement* SteelSupportPhys = new G4PVPlacement(
					0,
					G4ThreeVector(SteelSupportOffsetX,0,SteelSupportOffsetZ),
					SteelSupportLogic,
					"SteelSupportPhys",
					logicWorld,
					false,
					0,
					true);

	SteelSupportLogic->SetVisAttributes(visAttribAl);
*/
}


/*//////////////////////////////////////////////////////////////////////////////////////////////////////
*/
	 //the "photocathode" is a metal slab at the front of the glass that
	 //is only an approximation of the real thing since it only
	 //absorbs or detects the photons based on the efficiency set

//////////////////// moving placement of phCathSolid from PMT to the      ////////////////////////
//////////////////// Bar volume s.t. there are no logical volume overlaps ///////////////////////

   //sensitive detector is not actually on the photocathode.
   //processHits gets done manually by the stepping action.
   //It is used to detect when photons hit and get absorbed&detected at the
   //boundary to the photocathode (which doesnt get done by attaching it to a
   //logical volume.
   //It does however need to be attached to something or else it doesnt get

    //==========================================================================
    //---------------Scintillator Sensitive Detector---------------
    //==========================================================================
/*
	G4SDManager* SDman = G4SDManager::GetSDMpointer();
	G4String SDnameC = "MilliQan/ScintiSensitiveDetector";
	mqScintSD* scintSD = new mqScintSD(SDnameC);
	SDman->AddNewDetector ( scintSD );

	plScin_logic->SetSensitiveDetector(scintSD);

	const int nbAllPmts = nBarXCount*nBarYCount;
  */  
   //reset at the begining of events
   //==========================================================================
   //---------------PMT Sensitive Detector---------------
   //==========================================================================
   G4String SDnameOfPMTDetector = "PMT_SD";
   mqPMTSD* myPMTSD = new mqPMTSD(SDnameOfPMTDetector);

   G4SDManager::GetSDMpointer()->AddNewDetector(myPMTSD);
//	myPMTSD->InitPMTs(nbAllPmts);
	myPMTSD->SetR878_QE(GetPMTEff_R878());
	myPMTSD->SetR7725_QE(GetPMTEff_R7725());
	myPMTSD->SetET9814B_QE(GetPMTEff_ET9814B());
	phCathLog->SetSensitiveDetector(myPMTSD); //change HERE for PMT Scenario

   //G4String SDnameOfScintDetector = "Rock_SD";
   G4String SDnameOfScintDetector = "Scint_SD";
   mqScintSD* myScintSD = new mqScintSD(SDnameOfScintDetector);

   G4SDManager::GetSDMpointer()->AddNewDetector(myScintSD);
//   	wrap_logic->SetSensitiveDetector(myScintSD);
//   	ScintSlabWrapLogic->SetSensitiveDetector(myScintSD);
//   	ScintPanelWrapLogic->SetSensitiveDetector(myScintSD);

//	myPMTSD->InitPMTs(nbAllPmts);
//	myPMTSD->SetR878_QE(GetPMTEff_R878());
//	myPMTSD->SetR7725_QE(GetPMTEff_R7725());
//	myPMTSD->SetET9814B_QE(GetPMTEff_ET9814B());
//	phCathLog->SetSensitiveDetector(myPMTSD); //change HERE for PMT Scenario


//  	G4ThreeVector PMTPosition1 = G4ThreeVector(0,pmtYoffset,scintZ+height_pmt/2+(nLayers-1)*layerSpacing/2);
  //  	G4RotationMatrix rotm1  = G4RotationMatrix();//(G4ThreeVector(0, -1., 0. ), pi / 2.0);
//	G4Transform3D transform = G4Transform3D(rotm1,PMTPosition1);
/*
	G4PVPlacement* pmtPhys = new G4PVPlacement(transform,   //rotation,position
			                     pmtLog,                   //its logical volume
			                     "pmt_physic",                //its name
			                     logicWorld,              //its mother  volume
			                     false,                    //no boolean operation
			                     nbAllPmts,             //copy number
			                     true);           //checking overlaps
*/
	// visual attributes for all PMTs;
	G4VisAttributes* visAttribPMT = new G4VisAttributes(G4Colour::Blue());
	visAttribPMT->SetForceWireframe(false);
	visAttribPMT->SetForceSolid(true);
	visAttribPMT->SetVisibility(true);

	//setting same color so it doesn't look bad in isometric view
	
	G4VisAttributes* visAttribPhCath = new G4VisAttributes(G4Colour::Blue());
	//G4VisAttributes* visAttribPhCath = new G4VisAttributes(G4Colour::Red());
	visAttribPhCath->SetForceWireframe(false);
	visAttribPhCath->SetForceSolid(false);
	visAttribPhCath->SetVisibility(true);

	pmtLog->SetVisAttributes(visAttribPMT);
	phCathLog->SetVisAttributes(visAttribPhCath);

	//==============================================================================
	// Optical Surfaces
	//==============================================================================
	// Optical Properties
	// Note that, as mentioned in HyperNews posts and the G4AppDev Guide, the definition
	// of a logical bordfer surface involves a ordered pair of materials, such that
	// despite there being no clearly defined structure/class in the prototpye, the
	// order in which the surfaces are listed defines the orientation of the surface;
	// particles "see" this surface only when traveling from the first listed physical
	// volume into the second, which is why I have created a pair of complementary
	// surfaces for every interface (producing identical, bidirectional interactions).


    G4OpticalSurface* opSDielectricBiAlkali = new G4OpticalSurface("Detector", unified,
                                                    polished, dielectric_metal);
    //////////////////
    G4MaterialPropertiesTable* mtphcath =  matBiAlkali->GetMaterialPropertiesTable();
    opSDielectricBiAlkali->SetMaterialPropertiesTable(mtphcath);
    opSDielectricBiAlkali->SetMaterialPropertiesTable(SetOpticalPropertiesOfPMT());

    G4OpticalSurface* opSWrapScintillator = new G4OpticalSurface("Wrapping", unified,
                              /*ground*/      groundteflonair,
                                                dielectric_metal);
	const G4int nEntriesWrap = 2;
	G4double photonEnergyWrap[nEntriesWrap]={ 1.5 * eV,6.3 * eV};
	G4double wrap_REFL[nEntriesWrap] = {wrapRefl,wrapRefl};//{0.95,0.95}
//	G4double wrap_RIND[nEntriesWrap];

	G4MaterialPropertiesTable* mptWrap = new G4MaterialPropertiesTable();
	//mptWrap->AddProperty("TRANSMITTANCE",photonEnergyWrap, foil_REFL, nEntriesWrap);//->SetSpline(true);
	mptWrap->AddProperty("REFLECTIVITY",photonEnergyWrap, wrap_REFL, nEntriesWrap);
    
    opSWrapScintillator->SetMaterialPropertiesTable(mptWrap);
////////////////// PMT non-photocath refl ///////////////////////////
    G4OpticalSurface* opSPMT = new G4OpticalSurface("PMT Dead Zone", unified,
                              /*ground*/      polished,
                                                dielectric_metal);
	G4double photonEnergyPMT[nEntriesWrap]={ 1.5 * eV,6.3 * eV};
	G4double wrap_PMT[nEntriesWrap] = {0,0};//{0.95,0.95}
//	G4double wrap_RIND[nEntriesWrap];

	G4MaterialPropertiesTable* mptPMT = new G4MaterialPropertiesTable();
	//mptWrap->AddProperty("TRANSMITTANCE",photonEnergyWrap, foil_REFL, nEntriesWrap);//->SetSpline(true);
	mptPMT->AddProperty("REFLECTIVITY",photonEnergyPMT, wrap_PMT, nEntriesWrap);
    
    opSPMT->SetMaterialPropertiesTable(mptPMT);
/////////////////////////////////////////////////////////////////////// 

//define optical surfaces for each object in the sim, and match the optical properties of each surface to it

    new G4LogicalSkinSurface("Wrap", wrap_logic, opSWrapScintillator);
    new G4LogicalSkinSurface("Wrap", ScintPanelWrapLogic, opSWrapScintillator);
    new G4LogicalSkinSurface("Wrap", ScintSlabWrapLogic, opSWrapScintillator);
    new G4LogicalSkinSurface("PhCath", phCathLog, opSDielectricBiAlkali);
    new G4LogicalSkinSurface("PMTDeadZone", pmtLog, opSPMT);

//Connect detector volume (photocathode) to each surface which is in contact with it
// use this if you want to explicitly define the allowed optical surfaces rather than a wrapping
//        new G4LogicalBorderSurface("Glass->PhCath",
//			pmtPhys,phCathPhys,opSDielectricBiAlkali);
//			PMTParam_phys,phCathPhys,opSDielectricBiAlkali);

//	new G4LogicalBorderSurface("PhCath->Glass",
//			phCathPhys,pmtPhys,opSDielectricBiAlkali);
//			phCathPhys,PMTParam_phys,opSDielectricBiAlkali);
/*
    new G4LogicalBorderSurface("Scinti->PhCath",
			barParamPhys[0], PMTParamPhys[0] ,opSDielectricBiAlkali);
			//plScin_physic, phCathPhys ,opSDielectricBiAlkali);

    new G4LogicalBorderSurface("PhCath->Scinti",
    			PMTParamPhys[0] ,barParamPhys[0] ,opSDielectricBiAlkali);
    			//phCathPhys ,plScin_physic ,opSDielectricBiAlkali);
*/
/*
    new G4LogicalBorderSurface("AirGap->PhCath",
			airgap_physic, phCathPhys ,opSDielectricBiAlkali);

    new G4LogicalBorderSurface("PhCath->AirGap",
    			phCathPhys ,airgap_physic ,opSDielectricBiAlkali);
*/
//*/


    return physicWorld;

}

void mqDetectorConstruction::SetMagField(G4double fieldValueX, G4double fieldValueY, G4double fieldValueZ) {

  fMonFieldSetup->SetMagField(fieldValueX, fieldValueY, fieldValueZ);

}

G4MaterialPropertiesTable* mqDetectorConstruction::SetOpticalPropertiesOfPS(){


G4MaterialPropertiesTable* mptPlScin = new G4MaterialPropertiesTable();

 const G4int nEntries= 43;//301;//100;

	G4double EJ200_SCINT[nEntries];
	G4double EJ200_RIND[nEntries];
	G4double EJ200_ABSL[nEntries];
	G4double photonEnergy[nEntries];

	std::ifstream ReadEJ200;
	G4int ScintEntry=0;
	G4String filler;
	G4double pEnergy;
	G4double pWavelength;
	G4double pSEff;
	G4cout << "mqOpticalFilePath is: " << mqOpticalFilePath << G4endl;
	ReadEJ200.open(mqOpticalFilePath+"EJ200ScintSpectrum.txt");
	if(ReadEJ200.is_open()){
	while(!ReadEJ200.eof()){
	ReadEJ200 >> pWavelength >> pSEff;
	pEnergy = (1240/pWavelength)*eV;
	photonEnergy[ScintEntry] = pEnergy;
	EJ200_SCINT[ScintEntry] = pSEff;
	G4cout << "read-in energy scint: " << photonEnergy[ScintEntry] << " eff: " << EJ200_SCINT[ScintEntry] << G4endl;
	ScintEntry++;
	}
	}
	else
	G4cout << "Error opening file: " << "EJ200ScintSpectrum.txt" << G4endl;
	ReadEJ200.close();


	for (int i = 0; i < nEntries; i++) {
		EJ200_RIND[i] = 1.58;//58; // refractive index at 425 nm
		//EJ200_ABSL[i] *= myPSAttenuationLength;
		EJ200_ABSL[i] = 3.8*m;//2.5 * m; // bulk attenuation at 425 nm
	}

	mptPlScin->AddProperty("FASTCOMPONENT", photonEnergy, EJ200_SCINT,
			nEntries);//->SetSpline(true);


	mptPlScin->AddProperty("ABSLENGTH", photonEnergy, EJ200_ABSL,
				nEntries);//->SetSpline(true);

	mptPlScin->AddConstProperty("SCINTILLATIONYIELD", 10000. / MeV); //--- according to EJ200
	mptPlScin->AddConstProperty("RESOLUTIONSCALE", 1.0);
	mptPlScin->AddConstProperty("FASTTIMECONSTANT", 2.1 * ns); //decay time, according to EJ200
	mptPlScin->AddProperty("RINDEX", photonEnergy, EJ200_RIND, nEntries);//->SetSpline(true);

return mptPlScin;
}

G4MaterialPropertiesTable* mqDetectorConstruction::SetOpticalPropertiesOfPMT(){

	G4MaterialPropertiesTable* mptPMT = new G4MaterialPropertiesTable();

	const G4int nEntriesPMT = 41;//27;//36;//2;//37;//15; //just figure out how many entries are in the text file,
								//unless you're not lazy like me and want to dynamically allocate memory
//* PMT QEff read-in (updated Jan 2018)
	G4double photonEnergyPMT[nEntriesPMT];
	G4double photocath_EFF[nEntriesPMT];
	G4double PhCath_REFL[nEntriesPMT]; // to be determined
	
	std::ifstream ReadPMTQEff;
	G4int PMTEntry=0;
	G4String filler;
	G4double pEnergy;
	G4double pWavelength;
	G4double pQEff;
	ReadPMTQEff.open(mqOpticalFilePath+"PMT_R878_QE_orig.txt");
	if(ReadPMTQEff.is_open()){
	while(!ReadPMTQEff.eof()){
	ReadPMTQEff >> pWavelength >> pQEff;
	pQEff=1;
	pEnergy = (1240/pWavelength)*eV;
	photonEnergyPMT[PMTEntry] = pEnergy;
	photocath_EFF[PMTEntry] = pQEff;
	PhCath_REFL[PMTEntry] = 0;
	G4cout << "read-in energy: " << photonEnergyPMT[PMTEntry] << " eff: " << photocath_EFF[PMTEntry] << G4endl;
	PMTEntry++;
	}
	}
	else
	G4cout << "Error opening file: " << "PMT_R878_QE.txt" << G4endl;
	ReadPMTQEff.close();
/*	
	G4double PhCath_REFL[nEntriesPMT] = { // to be determined
			0., 0.,0.,0.,0.,0.,0.,0.,0.,0.,
			0., 0.,0.,0.,0.//,0.,0.,0.,0.,0.,
			//0., 0.,0.,0.,0.,0.,0.//,0.,0.,0.,
			//0., 0.,0.,0.,0.,0.//,0.
			};
*/

	mptPMT->AddProperty("REFLECTIVITY", photonEnergyPMT,PhCath_REFL, nEntriesPMT);//->SetSpline(true);
	mptPMT->AddProperty("EFFICIENCY",photonEnergyPMT,photocath_EFF,nEntriesPMT);//->SetSpline(true);

	G4cout << "successfully at end of PMT optical table" << G4endl;
	
	return mptPMT;

}

G4PhysicsVector mqDetectorConstruction::GetPMTEff_R878(){
	
	std::ifstream ReadPMTQEff;
	ReadPMTQEff.open(mqOpticalFilePath+"PMT_R878_QE.txt");
	G4PhysicsVector effVec;
	effVec.Retrieve(ReadPMTQEff,true);
	if (effVec.GetVectorLength()!=0) G4cout << "Quantum Efficiency successfully retrieved for PMT_R878_QE" << G4endl;
	else G4cout << "ERROR: Vector length is zero!" << G4endl;
	effVec.ScaleVector(1,2); //multiplying by 2 for scaling purposes
	return effVec;
}

G4PhysicsVector mqDetectorConstruction::GetPMTEff_R7725(){
	
	std::ifstream ReadPMTQEff;
	ReadPMTQEff.open(mqOpticalFilePath+"PMT_R7725_QE.txt");
	G4PhysicsVector effVec;
	effVec.Retrieve(ReadPMTQEff,true);
	if (effVec.GetVectorLength()!=0) G4cout << "Quantum Efficiency successfully retrieved for PMT R7725" << G4endl;
	else G4cout << "ERROR: Vector length is zero!" << G4endl;
	effVec.ScaleVector(1,2); //multiplying by 2 for scaling purposes
	return effVec;
}

G4PhysicsVector mqDetectorConstruction::GetPMTEff_ET9814B(){
	std::ifstream ReadPMTQEff;
	ReadPMTQEff.open(mqOpticalFilePath+"PMT_ET9814B_QE.txt");
	G4PhysicsVector effVec;
	effVec.Retrieve(ReadPMTQEff,true);
	if (effVec.GetVectorLength()!=0) G4cout << "Quantum Efficiency successfully retrieved for PMT ET9814B" << G4endl;
	else G4cout << "ERROR: Vector length is zero!" << G4endl;
	effVec.ScaleVector(1,2); //multiplying by 2 for scaling purposes
	return effVec;
}

void mqDetectorConstruction::UpdateGeometry() {
	if (verbose >= 0) {
		G4cout << "MilliQan> Update geometry." << G4endl;
		G4cout
				<< "               Don't use this command explicitly, it's obsolete and can crash the run."
				<< G4endl;
	}
	// clean-up previous geometry
    G4GeometryManager::GetInstance()->OpenGeometry();
    G4PhysicalVolumeStore::GetInstance()->Clean();
    G4LogicalVolumeStore::GetInstance()->Clean();
    G4SolidStore::GetInstance()->Clean();
    G4LogicalSkinSurface::CleanSurfaceTable();
    G4LogicalBorderSurface::CleanSurfaceTable();
    G4SurfaceProperty::CleanSurfacePropertyTable();
    //define new one
	G4RunManager::GetRunManager()->DefineWorldVolume(Construct());
	G4RunManager::GetRunManager()->GeometryHasBeenModified();
	updated=false;
}


void mqDetectorConstruction::SetDefaults(){
  //Resets to default values


  updated=true;
}



