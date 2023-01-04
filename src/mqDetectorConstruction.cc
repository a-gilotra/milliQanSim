// created 22-04-2019
// author: Ryan Schmitz (UCSB)

//==============================================================================
#include "mqDetectorConstruction.hh"
#include "mqScintSD.hh"
#include "mqPMTSD.hh"
#include "mqUserEventInformation.hh"

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

const G4int nLayers = 1; //number of layers in detector. Up here since it defines an array
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
		G4cout << "BeamRad> Construct geometry." << G4endl;
	}
	G4double fWorld_x = 20.0 * m;
	G4double fWorld_y = 20.0 * m;
	G4double fWorld_z = 20.0 * m;

	G4Material* worldMaterial = G4NistManager::Instance()->FindOrBuildMaterial(
	//			"G4_Galactic");
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

///////////////////////////////    End world //////////////////////////////////////////////////
///////////////////////////////   Start Scintillator/Wrapping  ///////////////////////////////////////////
	//==============================================================================
	// size of Scintillator and wrapping
	//==============================================================================

	G4bool abs1=false;
	G4bool abs2=false;
	G4bool abs3=false;
	G4bool abs4=false;
	G4bool abs5=false;

	//G4double absThickness = 50*um;

	G4double layerSpacing = 1*mm;
/*
	G4double absThickness1 = 500*um;
	G4double absThickness2 = 500*um;
	G4double absThickness3 = 500*um;
	G4double absThickness4 = 20000*um;
*/	
	//G4double absThickness1 = 0.250*mm*25.4; //inches to mm;
	G4double absThickness1 = 1.0/2*mm; //inches to mm;
	G4double absThickness2 = 0.125*mm*25.4; //inches to mm;
	G4double absThickness3 = 0.064*mm*25.4; //inches to mm;
	G4double absThickness4 = 0.032*mm*25.4; //inches to mm;
	//G4double absThickness5 = 0.5*mm; //inches to mm;
	//G4double absThickness5 = 0.0347*mm; //inches to mm;
	G4double absThickness5 = 2*0.125*25.4*mm; //inches to mm;
	
	G4double scintVetoThickness = 5*cm;

	G4double measurementPos=10*mm;
	//G4double measurementPos=243*mm;
	//G4double measurementPos=0*mm;
	G4double wrapRefl = 0.99;//0.97;
	G4double scintX = 200/2*um;
	//G4double scintY = 50/2*mm;
	G4double scintY = 10/2*m;
	//G4double scintY = 2/2*cm;
	//G4double scintY = 2/2*mm;
	G4double scintXY = 1.3/2*mm;
	G4double scintZ  = 10/2*m;//(each of these dimensions represents the half-width; e.g. this is 600mm long)
	//G4double scintZ  = 2/2*cm;//(each of these dimensions represents the half-width; e.g. this is 600mm long)
	//G4double scintZ  = 2/2*mm;//(each of these dimensions represents the half-width; e.g. this is 600mm long)
	//G4double scintZ  = 50/2*mm;//(each of these dimensions represents the half-width; e.g. this is 600mm long)

	G4double SiBulkX = 1.0/2*m;
	//G4double SiBulkX = 2/2*cm;
	G4double PbX1 = 0.250/2*mm*25.4; //inches to mm
	G4double PbX2 = 0.125/2*mm;
	G4double PbX3 = 0.064/2*mm;
	G4double PbX4 = 0.032/2*mm;
	G4double detX = 2/2*cm;
	//G4double detX = 50/2*um;

	//3cm between detector and L1, 1cm between adjacent layers, source always positioned at L5
	//L1: 3
	//L2: 4
	//L3: 5
	//L4: 6
	//L5: 7
	
	G4double L1 = 2*cm;
	G4double L2 = 3*cm;
	G4double L3 = 4*cm;
	G4double L4 = 5*cm;
	G4double L5 = 6*cm;
	
	G4double airGapThickness = 0.3*mm; //1*mm
	G4double wrapThickness = 1*mm; //3*mm
	G4double airgapX = scintX+airGapThickness;	
	G4double airgapY = scintY+airGapThickness;	
	G4double airgapXY = scintXY+airGapThickness;	
	G4double airgapZ = scintZ+airGapThickness;
	G4double wrapX=airgapX+wrapThickness;
	G4double wrapY=airgapY+wrapThickness;
	G4double wrapXY=airgapXY+wrapThickness;
	G4double wrapZ=airgapZ+wrapThickness;

        G4double frontLayerMid = -132*cm;
        G4double midLayerMid = -10*cm;
        G4double backLayerMid = 115*cm;

	//this is here because the entire geometry is shifted 6cm upwards
        G4double centerOffsetX = 6*cm;

	//measured from the centers; the gap between the bars
	G4double barSpacingXY = 60*mm;
	//G4double layerSpacing = 1000*mm; 
	//in visualization this gets swapped, so this is actually 3x2 rather than 2x3
        G4int nBarXCount = 1; //number of bars in grid, so this is NxN
        G4int nBarYCount = 1; //number of bars in grid, so this is NxN
	this->SetNBarPerLayer(nBarXCount*nBarYCount);
	this->SetNLayer(nLayers);
 
	 G4double outerRadius_pmt   = 52* mm /2.; //26 //52
	 G4double outerRadius_cath  = 52* mm /2.; //26 //52
	 G4double height_pmt        = 100* mm; //200.*mm;
	 G4double height_cath       = 2.0 * mm; //2mm
	 G4double pmtYoffset        = 0*mm;
	 G4double muMetalThickness = 0.1*mm;

	 //G4double worldRotation = -43.1*deg;
	 G4double worldRotation = 0*deg;
	
	//overallX = bottom half of detector below 3rd Si Layer
	G4double overallBotX = layerSpacing*5+absThickness3+absThickness4+scintVetoThickness+2.5*2*scintX+1*mm;
	G4double overallY = scintY;
	G4double overallZ = scintZ;

	//overallTopX = top half of detector above 3rd Si layer
	G4double overallTopX = layerSpacing*4+absThickness2+absThickness1+2.5*2*scintX+1*mm;
	

	G4double steelThickness = 0.5*cm;
	G4double steelY = scintY*3;
	G4double steelZ = scintZ*3;

	G4double cuRodRadius = 0.5*2.54*cm;
	G4double cuSpacing = 5*cm;
	
	G4NistManager* nistMan = G4NistManager::Instance();

///////////////////////////////////////////////////////////////////////////////


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
	G4Element* elI = nistMan->FindOrBuildElement("I");
	G4Element* elNa = nistMan->FindOrBuildElement("Na");
	G4Element* elSi = nistMan->FindOrBuildElement("Si");
	G4Element* elAl = nistMan->FindOrBuildElement("Al");
	G4Element* elAu = nistMan->FindOrBuildElement("Au");
        G4Element* elCs = nistMan->FindOrBuildElement("Cs"); // Cesium;
        G4Element* elSb = nistMan->FindOrBuildElement("Sb"); // Antimony;
        G4Element* elB = nistMan->FindOrBuildElement("B"); // Boron;
        G4Element* elLa = nistMan->FindOrBuildElement("La");
        G4Element* elLu = nistMan->FindOrBuildElement("Lu");
        G4Element* elY = nistMan->FindOrBuildElement("Y");
        G4Element* elRh = nistMan->FindOrBuildElement("Rh");
        G4Element* elTe = nistMan->FindOrBuildElement("Te");
        G4Element* elCd = nistMan->FindOrBuildElement("Cd");
        G4Element* elGd = nistMan->FindOrBuildElement("Gd");
        G4Element* elZn = nistMan->FindOrBuildElement("Zn");
        G4Element* elCu = nistMan->FindOrBuildElement("Cu");
        G4Element* elPb = nistMan->FindOrBuildElement("Pb");
	
	//mu metal materials	
	G4Element* elNi = nistMan->FindOrBuildElement("Ni"); //Nickel;
	G4Element* elFe = nistMan->FindOrBuildElement("Fe"); //Iron;
	
	//there are other parts, but to first order this is 80% Ni, 15% Fe, 5% other stuff, so I'm just going to lump that into Fe
	G4Material* muMetal = new G4Material("muMetal",8.7*g/cm3, 2);
	muMetal->AddElement(elNi,0.8); //80% mass fraction
	muMetal->AddElement(elFe,0.2); //20% mass fraction


	G4Element* elPlastic = nistMan->FindOrBuildElement("G4_POLYSTYRENE");
//        G4Material* concreteMat = nistMan->FindOrBuildMaterial("G4_CONCRETE");
	//==========================================================================
	// BC-400 plastic scintillator; according to datasheet, elH = 1.103 *elC, elH + elC = 1
	//For the references for the optical properties see ../ref/EmissionSpectrum_BC-400.pdf

	//==========================================================================
	G4Material* wrapMat = nistMan->FindOrBuildMaterial("G4_POLYETHYLENE");
	G4Material* airgapMat = worldMaterial; //The air gap mat should be air, change this if you change worldmat to be vacuum/galactic		
	G4Material* AlMat = nistMan->FindOrBuildMaterial("G4_Al");
	G4Material* CuMat = nistMan->FindOrBuildMaterial("G4_Cu");
	G4Material* AgMat = nistMan->FindOrBuildMaterial("G4_Ag");
	G4Material* AuMat = nistMan->FindOrBuildMaterial("G4_Au");
	G4Material* FeMat = nistMan->FindOrBuildMaterial("G4_Fe");
	G4Material* CdMat = nistMan->FindOrBuildMaterial("G4_Cd");
	
	G4Material* NiMat = nistMan->FindOrBuildMaterial("G4_Ni");
	G4Material* PbMat = nistMan->FindOrBuildMaterial("G4_Pb");
	
	G4Material* SiMat = nistMan->FindOrBuildMaterial("G4_Si");
	
	G4Material* RhMat = nistMan->FindOrBuildMaterial("G4_Rh");
	G4Material* TeMat = nistMan->FindOrBuildMaterial("G4_Te");
	G4Material* GdMat = nistMan->FindOrBuildMaterial("G4_Gd");
	//G4Material* BrassMat = nistMan->FindOrBuildMaterial("G4_Brass");

	G4Material* steelMat = nistMan->FindOrBuildMaterial("G4_STAINLESS-STEEL");
	//going to use concrete as a first order approximation to rock, since it's close and we just want something dense
	G4Material* concreteMat = nistMan->FindOrBuildMaterial("G4_CONCRETE");
/*	
	G4Material* silicaMat = new G4Material("SiO2", 2.65 * g / cm3, 2);
	silicaMat->AddElement(elSi, 1);
	silicaMat->AddElement(elO, 2);
*/
        G4Material* matBiAlkali = new G4Material("matBiAlkali", 1.3 * g / cm3, 3);
        matBiAlkali->AddElement(elK, 2);
        matBiAlkali->AddElement(elCs, 1);
        matBiAlkali->AddElement(elSb, 1);


	G4Material* LaOMat = new G4Material("La2O3", 6.51 * g / cm3, 2);
	LaOMat->AddElement(elLa, 2);
	LaOMat->AddElement(elO, 3);
        
	G4Material* matPlScin = new G4Material("plScintillator", 1.032 * g / cm3, 2);
	//G4Material* matPlScin = new G4Material("plScintillator", 1.032 * g / cm3, 2);
        //matPlScin->AddElement(elC, .4755);
        //matPlScin->AddElement(elH, .5245);
        matPlScin->AddElement(elC, 10);
        matPlScin->AddElement(elH, 11);
	

//Pb
G4double density = 1.2*g/cm3;
G4int numel;
G4int natoms;
G4double fractionMass;
 G4Material* Epoxy = new G4Material("Epoxy" , density, numel=2);
 Epoxy->AddElement(elH, natoms=2);
 Epoxy->AddElement(elC, natoms=2);
 //SiO2 (Quarz)
 G4Material* SiO2 = new G4Material("SiO2",density= 2.200*g/cm3, numel=2);
 SiO2->AddElement(elSi, natoms=1);
 SiO2->AddElement(elO , natoms=2);
 //Pb (Glass + Epoxy)
 density = 1.86*g/cm3;
// G4Material* Pb = new G4Material("Pb" , density, numel=2);
// Pb->AddMaterial(Epoxy, fractionMass=0.472);
// Pb->AddMaterial(SiO2, fractionMass=0.528);
 
 G4Material* LYSO = new G4Material("LYSO" , density=7.10*g/cm3, numel=4);
 LYSO->AddElement(elLu, fractionMass=0.7145);
 LYSO->AddElement(elY, fractionMass=0.0403);
 LYSO->AddElement(elSi, fractionMass=0.0637);
 LYSO->AddElement(elO, fractionMass=0.1815);

 G4Material* NaI = new G4Material("NaI" , density=3.67*g/cm3, numel=2);
 NaI->AddElement(elNa, natoms=1);
 NaI->AddElement(elI, natoms=1);

 G4Material* wood = new G4Material("wood", density=0.9*g/cm3, numel=3);
 wood->AddElement(elH , 4);
 wood->AddElement(elO , 1);
 wood->AddElement(elC , 2);

 G4Material* BrassMat = new G4Material("BrassMat", density=8.52*g/cm3, numel=3);
 BrassMat->AddElement(elCu , fractionMass=0.62);
 BrassMat->AddElement(elZn , fractionMass=0.35);
 BrassMat->AddElement(elPb , fractionMass=0.03);

        //NaI->SetMaterialPropertiesTable(SetOpticalPropertiesOfLYSO());
        NaI->SetMaterialPropertiesTable(SetOpticalPropertiesOfNaI());
        LYSO->SetMaterialPropertiesTable(SetOpticalPropertiesOfLYSO());

	//NaI->GetIonisation()->SetBirksConstant(0.126*mm/MeV); // according to L. Reichhart et al., Phys. Rev, use (0.149*mm/MeV) for neutrons, but otherwise use 0.126

////////////////////////////////////////////////////////////////////////////////
//									     ///
//				Si Base					     ///
//									     ///
////////////////////////////////////////////////////////////////////////////////
/*
        G4Box* plastic_solid = new G4Box("plastic_solid",
                      outerRadius_NaI*2+NaI_spacing,
                      //NaI_spacing/2-PbThickness,
                      outerRadius_NaI,
                      height_NaI);

        G4LogicalVolume* plastic_logic = new G4LogicalVolume(
			plastic_solid,
			matPlScin,
			"plastic_logic",
			0, 0, 0);
	G4PVPlacement* plastic_physic = new G4PVPlacement(
                        0,
                        G4ThreeVector(0,0,0),
                        plastic_logic,
                        "plastic_physic",
                        logicWorld,
                        false,
                        0,
                        true);
*/
	G4double AlThickness=2.5*mm;
	G4double holdershift = 1*cm;
        G4double outerRadius_NaISmall=(25.5*mm/2+AlThickness);
        G4double height_NaISmall=30.0*mm/2+AlThickness/2;
        G4double outerRadius_NaILarge=(41.0*mm/2+AlThickness);
        G4double height_NaILarge=(44.0*mm/2+AlThickness/2);
	G4double width_LYSOArray=(21.2*mm/2); //square, this is both dims
	G4double length_LYSOArray=(22.0*mm/2);
	G4double height_holder=120*mm/2;
	G4double width_holder=120*mm/2;
	G4double length_holder=170*mm/2;
        G4double spacing=64.0*mm/2;
	G4double shift = length_holder-height_NaILarge-wrapThickness/2-airGapThickness/2;
 	G4double caseX = 45*cm;
 	G4double caseY = 34*cm;
 	G4double caseZ = 13*cm;
        G4double caseThick = 2*mm;
	G4double plasticThick = 0.5*cm;

        G4Box* case_solid_out = new G4Box("case_solid_out",
                      caseX/2+caseThick,
                      caseY/2+caseThick,
                      caseZ/2+caseThick);
        
	G4Box* case_solid_in = new G4Box("case_solid_in",
                      caseX/2,
                      caseY/2,
                      caseZ/2);
	G4SubtractionSolid* case_solid = new G4SubtractionSolid("case_solid",
			case_solid_out,
			case_solid_in);

        G4LogicalVolume* case_logic = new G4LogicalVolume(
			case_solid,
			wood,
			"case_logic",
			0, 0, 0);
///*
	G4PVPlacement* case_physic = new G4PVPlacement(
                        0,
                        G4ThreeVector(0,0,0),
                        case_logic,
                        "case_physic",
                        logicWorld,
                        false,
                        0,
                        true);
//*/
	G4Box* holderSolid = new G4Box("holderSolid",
                      width_holder,
                      height_holder,
                      length_holder);
	
	G4LogicalVolume* holder_logic = new G4LogicalVolume(
			holderSolid,
			worldMaterial,
			//matPlScin,
			"holder_logic",
			0, 0, 0);

	G4RotationMatrix* rot = new G4RotationMatrix();
	rot->rotateX(-90*degree);
	rot->rotateY(-90*degree);
	
	G4RotationMatrix* rotsub = new G4RotationMatrix();
	
	G4PVPlacement* holder_physic = new G4PVPlacement(
                        rot,
                        G4ThreeVector(holdershift,0,0),
                        holder_logic,
                        "holder_physic",
                        logicWorld,
                        false,
                        0,
                        true);

	G4Tubs* wrapNaILargeSolid = new G4Tubs("wrapNaILargeSolid",
                        0,
                        outerRadius_NaILarge+airGapThickness+wrapThickness,
			height_NaILarge+wrapThickness/2+airGapThickness/2,
                        0*deg,
                        360*deg);

	G4Tubs* wrapNaISmallSolid = new G4Tubs("wrapNaISmallSolid",
                        0,
                        outerRadius_NaISmall+airGapThickness+wrapThickness,
			height_NaISmall+wrapThickness/2+airGapThickness/2,
                        0*deg,
                        360*deg);

	G4Box* wrapLYSOSolid = new G4Box("wrapLYSOSolid",
                        width_LYSOArray+airGapThickness+wrapThickness,
                        width_LYSOArray+airGapThickness+wrapThickness,
                        length_LYSOArray+airGapThickness/2+wrapThickness/2);
	
	G4Tubs* plasticNaILargeSolid = new G4Tubs("plasticNaILargeSolid",
                        outerRadius_NaILarge+airGapThickness+wrapThickness,
                        outerRadius_NaILarge+airGapThickness+wrapThickness+plasticThick,
			height_NaILarge+wrapThickness/2+airGapThickness/2,
                        0*deg,
                        360*deg);

	G4Tubs* plasticNaISmallSolid = new G4Tubs("plasticNaISmallSolid",
                        outerRadius_NaISmall+airGapThickness+wrapThickness,
                        outerRadius_NaISmall+airGapThickness+wrapThickness+plasticThick,
			height_NaISmall+wrapThickness/2+airGapThickness/2,
                        0*deg,
                        360*deg);

	G4Box* plasticLYSOSolidOut = new G4Box("plasticLYSOSolidOut",
                        width_LYSOArray+airGapThickness+wrapThickness+plasticThick,
                        width_LYSOArray+airGapThickness+wrapThickness+plasticThick,
                        length_LYSOArray+airGapThickness/2+wrapThickness/2);

        G4SubtractionSolid* plasticLYSOSolid = new G4SubtractionSolid("plasticLYSOSolid",
                                plasticLYSOSolidOut,
                                wrapLYSOSolid,
                                rotsub,
                                G4ThreeVector(0,0,0));
                                //G4ThreeVector(0,0,0));

	G4Tubs* airgapNaILargeSolid = new G4Tubs("airgapNaILargeSolid",
                        0,
                        outerRadius_NaILarge+airGapThickness-AlThickness,
			height_NaILarge+airGapThickness/2-AlThickness/2,
                        0*deg,
                        360*deg);

	G4Tubs* airgapNaISmallSolid = new G4Tubs("airgapNaISmallSolid",
                        0,
                        outerRadius_NaISmall+airGapThickness-AlThickness,
			height_NaISmall+airGapThickness/2-AlThickness/2,
                        0*deg,
                        360*deg);

	G4Box* airgapLYSOSolid = new G4Box("airgapLYSOSolid",
                        width_LYSOArray+airGapThickness,
                        width_LYSOArray+airGapThickness,
                        length_LYSOArray+airGapThickness/2);

        G4LogicalVolume* plasticNaILarge_logic = new G4LogicalVolume(
                        plasticNaILargeSolid,
                        matPlScin,
                        "plasticNaILarge_logic",
                        0, 0, 0);

        G4LogicalVolume* plasticNaISmall_logic = new G4LogicalVolume(
                        plasticNaISmallSolid,
                        matPlScin,
                        "wrapNaISmall_logic",
                        0, 0, 0);
        
	G4LogicalVolume* plasticLYSO_logic = new G4LogicalVolume(
                        plasticLYSOSolid,
                        matPlScin,
                        "plasticLYSO_logic",
                        0, 0, 0);

        G4LogicalVolume* wrapNaILarge_logic = new G4LogicalVolume(
                        wrapNaILargeSolid,
                        wrapMat,
                        "wrapNaILarge_logic",
                        0, 0, 0);

        G4LogicalVolume* wrapNaISmall_logic = new G4LogicalVolume(
                        wrapNaISmallSolid,
                        wrapMat,
                        "wrapNaISmall_logic",
                        0, 0, 0);
        
	G4LogicalVolume* wrapLYSO_logic = new G4LogicalVolume(
                        wrapLYSOSolid,
                        wrapMat,
                        "wrapLYSO_logic",
                        0, 0, 0);

	G4LogicalVolume* airgapNaILarge_logic = new G4LogicalVolume(
                        airgapNaILargeSolid,
                        airgapMat,
                        "airgapNaILarge_logic",
                        0, 0, 0);

        G4LogicalVolume* airgapNaISmall_logic = new G4LogicalVolume(
                        airgapNaISmallSolid,
                        airgapMat,
                        "airgapNaISmall_logic",
                        0, 0, 0);
        
	G4LogicalVolume* airgapLYSO_logic = new G4LogicalVolume(
                        airgapLYSOSolid,
                        airgapMat,
                        "airgapLYSO_logic",
                        0, 0, 0);
///*
	G4PVPlacement* plasticNaISmall_physic1 = new G4PVPlacement(
                        0,
                        G4ThreeVector(-spacing,-spacing,shift+height_NaILarge-height_NaISmall),
                        //G4ThreeVector(-spacing,-spacing,shift+height_NaILarge-height_NaISmall+wrapThickness/2+airGapThickness/2),
                        plasticNaISmall_logic,
                        "NaISmallPlastic_physic1",
                        //logicWorld,
                        holder_logic,
                        false,
                        2,
                        true);

	G4PVPlacement* plasticNaISmall_physic2 = new G4PVPlacement(
                        0,
                        G4ThreeVector(spacing,spacing,shift+height_NaILarge-height_NaISmall),
                        plasticNaISmall_logic,
                        "NaISmallPlastic_physic2",
                        //logicWorld,
                        holder_logic,
                        false,
                        3,
                        true);
        G4PVPlacement* plasticNaILarge_physic = new G4PVPlacement(
                        0,
                        G4ThreeVector(spacing,-spacing,shift),
                        plasticNaILarge_logic,
                        "NaILargePlastic_physic",
                        //logicWorld,
                        holder_logic,
                        false,
                        1,
                        true);
        G4PVPlacement* plasticLYSO_physic = new G4PVPlacement(
                        0,
                        G4ThreeVector(-spacing,spacing,shift+height_NaILarge-length_LYSOArray),
                        plasticLYSO_logic,
                        "LYSOPlastic_physic",
                        //logicWorld,
                        holder_logic,
                        false,
                        4,
                        true);
//*/
///*
        G4PVPlacement* wrapNaISmall_physic1 = new G4PVPlacement(
                        0,
                        G4ThreeVector(-spacing,-spacing,shift+height_NaILarge-height_NaISmall),
                        //G4ThreeVector(-spacing,-spacing,shift+height_NaILarge-height_NaISmall+wrapThickness/2+airGapThickness/2),
                        wrapNaISmall_logic,
                        "NaISmallWrap_physic1",
                        //logicWorld,
                        holder_logic,
                        false,
                        2,
                        true);

        G4PVPlacement* wrapNaISmall_physic2 = new G4PVPlacement(
                        0,
                        G4ThreeVector(spacing,spacing,shift+height_NaILarge-height_NaISmall),
                        wrapNaISmall_logic,
                        "NaISmallWrap_physic2",
                        //logicWorld,
                        holder_logic,
                        false,
                        3,
                        true);
        G4PVPlacement* wrapNaILarge_physic = new G4PVPlacement(
                        0,
                        G4ThreeVector(spacing,-spacing,shift),
                        wrapNaILarge_logic,
                        "NaILargeWrap_physic",
                        //logicWorld,
                        holder_logic,
                        false,
                        1,
                        true);
        G4PVPlacement* wrapLYSO_physic = new G4PVPlacement(
                        0,
                        G4ThreeVector(-spacing,spacing,shift+height_NaILarge-length_LYSOArray),
                        wrapLYSO_logic,
                        "LYSOWrap_physic",
                        //logicWorld,
                        holder_logic,
                        false,
                        4,
                        true);

//*/

	G4PVPlacement* airgapNaISmall_physic = new G4PVPlacement(
                        0,
                        G4ThreeVector(0,0,-wrapThickness/2-AlThickness/2),
                        airgapNaISmall_logic,
                        "NaISmallAirgap_physic",
                        wrapNaISmall_logic,
                        //holder_logic,
                        false,
                        0,
                        true);
        G4PVPlacement* airgapNaILarge_physic = new G4PVPlacement(
                        0,
                        G4ThreeVector(0,0,-wrapThickness/2-AlThickness/2),
                        airgapNaILarge_logic,
                        "NaILargeAirgap_physic",
                        wrapNaILarge_logic,
                        //holder_logic,
                        false,
                        0,
                        true);
        G4PVPlacement* airgapLYSO_physic = new G4PVPlacement(
                        0,
                        G4ThreeVector(0,0,-wrapThickness/2),
                        airgapLYSO_logic,
                        "LYSOAirgap_physic",
                        wrapLYSO_logic,
                        //holder_logic,
                        false,
                        0,
                        true);

	G4Box* LYSOSolid = new G4Box("LYSOSolid",
                      width_LYSOArray,
                      width_LYSOArray,
                      length_LYSOArray);

	G4Tubs* NaISmallSolid = new G4Tubs(
                         "NaISmallSolid",
                         0,
                         outerRadius_NaISmall-AlThickness,
                         height_NaISmall-AlThickness/2,
                         0*deg,
                         360*deg);

	G4Tubs* NaILargeSolid = new G4Tubs(
                         "NaILargeSolid",
                         0,
                         outerRadius_NaILarge-AlThickness,
                         height_NaILarge-AlThickness/2,
                         0*deg,
                         360*deg);
	
	G4LogicalVolume* LYSO_logic = new G4LogicalVolume(
				LYSOSolid,
				LYSO,
				"LYSO_logic",
				0, 0, 0);

	G4LogicalVolume* NaISmall_logic = new G4LogicalVolume(
				NaISmallSolid,
				NaI,
				"NaISmall_logic",
				0, 0, 0);
	
	G4LogicalVolume* NaILarge_logic = new G4LogicalVolume(
				NaILargeSolid,
				NaI,
				"NaILarge_logic",
				0, 0, 0);
        

	G4PVPlacement* NaISmall_physic = new G4PVPlacement(
                        0,
                        G4ThreeVector(0,0,-airGapThickness/2),
                        NaISmall_logic,
                        "NaISmall_physic",
			airgapNaISmall_logic,
                        //holder_logic,
                        false,
                        0,
                        true);
	G4PVPlacement* NaILarge_physic = new G4PVPlacement(
                        0,
                        G4ThreeVector(0,0,-airGapThickness/2),
                        NaILarge_logic,
                        "NaILarge_physic",
			airgapNaILarge_logic,
                        //holder_logic,
                        false,
                        0,
                        true);
	G4PVPlacement* LYSO_physic = new G4PVPlacement(
                        0,
                        G4ThreeVector(0,0,-airGapThickness/2),
                        LYSO_logic,
                        "LYSO_physic",
			airgapLYSO_logic,
                        //holder_logic,
                        false,
                        0,
                        true);
// plastic source
	G4double xdist = 72*mm;
	G4double sourceRadius = 1*2.54*cm/2;
	G4double sourceThickness = 0.445/2*cm;
	//G4double sourceThickness = 0.5/2*cm;
	//G4double sourceThickness = 0.125*2.54*cm/2*1.5;
	//G4double sourceRadius = 1*2.54*cm/2;
	//G4double sourceThickness = 0.125*2.54*cm/2;
	        G4Tubs* SourceSolid = new G4Tubs(
                         "SourceSolid",
                         0,
                         sourceRadius, //radius
                         sourceThickness, //thickness
                         0*deg,
                         360*deg);

	G4LogicalVolume* Source_logic = new G4LogicalVolume(
				SourceSolid,
				worldMaterial,
				//steelMat,
				//matPlScin,
				//CuMat,
				//AlMat,
				//PbMat,
				//BrassMat,
				"Source_logic",
				0, 0, 0);

	G4RotationMatrix* sourceRot = new G4RotationMatrix();
	sourceRot->rotateX(-90*degree);
	sourceRot->rotateY(-90*degree);

	G4PVPlacement* Source_physic = new G4PVPlacement(
                        //0,
                        sourceRot,
                        //G4ThreeVector(caseX/2+caseThick+sourceThickness-7*cm,-3*cm,caseZ/2+caseThick+sourceThickness),
                        G4ThreeVector(caseX/2+caseThick+sourceThickness,0,0),
			Source_logic,
                        "Source_physic",
                        //plastic_logic,
                        logicWorld,
                        false,
                        0,
                        true);

	G4Tubs* AlCapSmallSolid = new G4Tubs(
                         "AlCapSmallSolid",
                         0,
                         outerRadius_NaISmall,
                         AlThickness,
                         0*deg,
                         360*deg);

	G4Tubs* AlCapLargeSolid = new G4Tubs(
                         "AlCapLargeSolid",
                         0,
                         outerRadius_NaILarge,
                         AlThickness,
                         0*deg,
                         360*deg);
	
        G4Tubs* AlSmallSolid = new G4Tubs(
                         "AlSmallSolid",
                         outerRadius_NaISmall-AlThickness+wrapThickness+airGapThickness,
                         outerRadius_NaISmall+wrapThickness+airGapThickness,
                         height_NaISmall,
                         0*deg,
                         360*deg);
        G4Tubs* AlLargeSolid = new G4Tubs(
                         "AlLargeSolid",
                         outerRadius_NaILarge-AlThickness+wrapThickness+airGapThickness,
                         outerRadius_NaILarge+wrapThickness+airGapThickness,
                         height_NaILarge,
                         0*deg,
                         360*deg);

	G4LogicalVolume* AlSmall_logic = new G4LogicalVolume(
				AlSmallSolid,
				AlMat,
				"AlSmall_logic",
				0, 0, 0);
	
	G4LogicalVolume* AlLarge_logic = new G4LogicalVolume(
				AlLargeSolid,
				AlMat,
				"AlLarge_logic",
				0, 0, 0);

	G4LogicalVolume* AlCapSmall_logic = new G4LogicalVolume(
				AlCapSmallSolid,
				AlMat,
				"AlCapSmall_logic",
				0, 0, 0);
	
	G4LogicalVolume* AlCapLarge_logic = new G4LogicalVolume(
				AlCapLargeSolid,
				AlMat,
				"AlCapLarge_logic",
				0, 0, 0);
///*
	G4PVPlacement* AlSmall_physic = new G4PVPlacement(
                        0,
                        G4ThreeVector(0,0,0),
                        AlSmall_logic,
                        "AlSmall_physic",
                        wrapNaISmall_logic,
                        false,
                        0,
                        true);
	G4PVPlacement* AlLarge_physic = new G4PVPlacement(
                        0,
                        G4ThreeVector(0,0,0),
                        AlLarge_logic,
                        "AlLarge_physic",
                        wrapNaILarge_logic,
                        false,
                        0,
                        true);
	G4PVPlacement* AlCapSmall_physic1 = new G4PVPlacement(
                        0,
                        G4ThreeVector(0,0,height_NaISmall-AlThickness/2+wrapThickness+airGapThickness),
                        AlCapSmall_logic,
                        "AlCapSmall_physic1",
                        wrapNaISmall_logic,
                        false,
                        0,
                        true);
	G4PVPlacement* AlCapLarge_physic1 = new G4PVPlacement(
                        0,
                        G4ThreeVector(0,0,height_NaILarge-AlThickness/2+wrapThickness+airGapThickness),
                        AlCapLarge_logic,
                        "AlCapLarge_physic1",
                        wrapNaILarge_logic,
                        false,
                        0,
                        true);
//*/
////////////////////////////////////////////////////////////////////////////////
//									     ///
//	         		Steel Wall				     ///
//									     ///
////////////////////////////////////////////////////////////////////////////////


//	G4Box* steelWall_solid = new G4Box("steelWall_solid",
//			steelThickness/2,
//			steelY,
//			steelZ);
/*
	G4Tubs* steelWall_solid = new G4Tubs("steelWall_solid",
			10*cm,
			10*cm+steelThickness,
			steelZ*10,
			0*deg,
			360*deg);

	G4LogicalVolume* steelWall_logic = new G4LogicalVolume(
			steelWall_solid,
			steelMat,
			"steelWall_logic",
			0,0,0);

        G4PVPlacement* steelWall_physic = new G4PVPlacement(
                        0,
                        G4ThreeVector(12*cm,0,0),
                        steelWall_logic,
                        "steelWall_physic",
                        logicWorld,
                        false,
                        0,
                        true);

////////////////////////////////////////////////////////////////////////////////
//									     ///
//				Copper Rod				     ///
//									     ///
////////////////////////////////////////////////////////////////////////////////

	G4Tubs* cuRod_solid = new G4Tubs("cuRod_solid",
			0,
			cuRodRadius,
			steelZ,
			0*deg,
			360*deg);

	G4LogicalVolume* cuRod_logic = new G4LogicalVolume(
			cuRod_solid,
			CuMat,
			"CuRod_logic",
			0,0,0);

	G4PVPlacement* cuRod_physic = new G4PVPlacement(
			0,
			G4ThreeVector(cuSpacing,0,0),
			cuRod_logic,
			"cuRod_physic",
			logicWorld,
			false,
			0,
			true);

//////////////////////////////////////////////////////////////////////////
//									//
//			Overall Detector Geometry		     	//
//									//
//////////////////////////////////////////////////////////////////////////

	G4Box* SiOverall_solid = new G4Box("SiOverall_solid",
			overallBotX, //using because it's thicker/sure to contain
			overallY,
			overallZ);

	G4LogicalVolume* SiOverall_logic = new G4LogicalVolume(
			SiOverall_solid,
			worldMaterial,
			"SiOverall_logic",
			0,0,0);

        G4PVPlacement* SiOverall_physic = new G4PVPlacement(
                        0,
                        G4ThreeVector(-overallBotX-steelThickness,0,0),
                        SiOverall_logic,
                        "SiOverall_physic",
                        logicWorld,
                        false,
                        0,
                        true);

	SiOverall_logic->SetVisAttributes(visAttWorld);
*/
/////////////////////////////////////////////////////////////////////////////////
//                            Wrapping Geometry                                //
/////////////////////////////////////////////////////////////////////////////////	
/* //geometry to be used when photocathode is attached to PMT
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
	G4RotationMatrix* rot = new G4RotationMatrix();
	G4SubtractionSolid* wrap_solid = new G4SubtractionSolid("wrap_solid",
			wrap_solid_total,
			wrap_PMT_hole,
			rot,
			//11 mm is y-axis PMT offset
			G4ThreeVector(0,0,wrapZ-(airGapThickness+wrapThickness)/2));
*/

/*
	G4Box* wrap_solid = new G4Box("wrap_solid",
			wrapXY,
			wrapXY,
			wrapZ);

	G4LogicalVolume* wrap_logic = new G4LogicalVolume(
			wrap_solid,
			wrapMat,
			"wrap_logic",
			0, 0, 0);


	
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
//                            Scintillator Geometry                                //
/////////////////////////////////////////////////////////////////////////////////	
/*
	G4Box* SiDet_solid = new G4Box("SiDet_solid",
				scintX, 
				scintY,
				scintZ);

	G4LogicalVolume* SiDet_logic1 = new G4LogicalVolume(
				SiDet_solid,
				//matPlScin,
				SiMat,
				"SiDet_logic1",
				0, 0, 0);

	G4LogicalVolume* SiDet_logic2 = new G4LogicalVolume(
				SiDet_solid,
				//matPlScin,
				SiMat,
				"SiDet_logic2",
				0, 0, 0);

	G4LogicalVolume* SiDet_logic3 = new G4LogicalVolume(
				SiDet_solid,
				//matPlScin,
				SiMat,
				"SiDet_logic3",
				0, 0, 0);

	G4LogicalVolume* SiDet_logic4 = new G4LogicalVolume(
				SiDet_solid,
				//matPlScin,
				SiMat,
				"SiDet_logic4",
				0, 0, 0);

	G4LogicalVolume* SiDet_logic5 = new G4LogicalVolume(
				SiDet_solid,
				//matPlScin,
				SiMat,
				"SiDet_logic5",
				0, 0, 0);

	G4PVPlacement* SiDet_physic1 = new G4PVPlacement(
			0,
			G4ThreeVector(2*wrapX*2+4*layerSpacing+absThickness1+absThickness2,0,0),
			SiDet_logic1,
			"SiDet_physic1",
			SiOverall_logic,
			false,
			0,
			true);

	G4PVPlacement* SiDet_physic2 = new G4PVPlacement(
			0,
			G4ThreeVector(wrapX*2+2*layerSpacing+absThickness2,0,0),
			SiDet_logic2,
			"SiDet_physic2",
			SiOverall_logic,
			false,
			0,
			true);

	G4PVPlacement* SiDet_physic3 = new G4PVPlacement(
			0,
			G4ThreeVector(),
			SiDet_logic3,
			"SiDet_physic3",
			SiOverall_logic,
			false,
			0,
			true);

	G4PVPlacement* SiDet_physic4 = new G4PVPlacement(
			0,
			G4ThreeVector(-wrapX*2-2*layerSpacing-absThickness3,0,0),
			SiDet_logic4,
			"SiDet_physic4",
			SiOverall_logic,
			false,
			0,
			true);
	G4PVPlacement* SiDet_physic5 = new G4PVPlacement(
			0,
			G4ThreeVector(-2*wrapX*2-4*layerSpacing-absThickness3-absThickness4,0,0),
			SiDet_logic5,
			"SiDet_physic5",
			SiOverall_logic,
			false,
			0,
			true);

	*/


//////////////////////////////////////////////////////////////////////////////////
//                           Absorber						//
//////////////////////////////////////////////////////////////////////////////////
/*

	G4Box* abs_solid1 = new G4Box("abs_solid1",
				absThickness1/2,
				scintY,
				scintZ);

	G4Box* abs_solid2 = new G4Box("abs_solid2",
				absThickness2/2,
				scintY,
				scintZ);

	G4Box* abs_solid3 = new G4Box("abs_solid3",
				absThickness3/2,
				scintY,
				scintZ);

	G4Box* abs_solid4 = new G4Box("abs_solid4",
				absThickness4/2,
				scintY,
				scintZ);

	G4Box* abs_solid5 = new G4Box("abs_solid5",
				absThickness5/2,
				scintY,
				scintZ);
	
	G4LogicalVolume* abs_logic1 = new G4LogicalVolume(
				abs_solid1,
				CdMat,//PbMat, //AlMat //absMat
				"abs_logic1",
				0, 0, 0);

	G4LogicalVolume* abs_logic2 = new G4LogicalVolume(
				abs_solid2,
				PbMat, //CuMat //absMat
				"abs_logic2",
				0, 0, 0);

	G4LogicalVolume* abs_logic3 = new G4LogicalVolume(
				abs_solid3,
				PbMat, //AgMat or NiMat //absMat
				"abs_logic3",
				0, 0, 0);

	G4LogicalVolume* abs_logic4 = new G4LogicalVolume(
				abs_solid4,
				PbMat, //FeMat //absMat
				//CuMat, //FeMat //absMat
				"abs_logic4",
				0, 0, 0);
	
	G4LogicalVolume* abs_logic5 = new G4LogicalVolume(
				abs_solid5,
				//matPlScin,//CuMat, //FeMat //absMat
				PbMat,//CuMat, //FeMat //absMat
				"abs_logic5",
				0, 0, 0);
        
	if(abs1){
	G4PVPlacement* abs_physic1 = new G4PVPlacement(
			0,
			G4ThreeVector(0,0,0),
			//G4ThreeVector(L1,0,0),
			abs_logic1,
			"abs_physic1",
			//SiOverall_logic,
			logicWorld,
			false,
			0,
			true);
	}
	if(abs2){
	G4PVPlacement* abs_physic2 = new G4PVPlacement(
			0,
			G4ThreeVector(L2,0,0),
			abs_logic2,
			"abs_physic2",
			//SiOverall_logic,
			logicWorld,
			false,
			0,
			true);
	}
	if(abs3){
	G4PVPlacement* abs_physic3 = new G4PVPlacement(
			0,
			G4ThreeVector(L3,0,0),
			abs_logic3,
			"abs_physic3",
			//SiOverall_logic,
			logicWorld,
			false,
			0,
			true);
	}
	if(abs4){
	G4PVPlacement* abs_physic4 = new G4PVPlacement(
			0,
			G4ThreeVector(L4,0,0),
			abs_logic4,
			"abs_physic4",
			//SiOverall_logic,
			logicWorld,
			false,
			0,
			true);
	}
	if(abs5){
	G4PVPlacement* abs_physic5 = new G4PVPlacement(
			0,
			//G4ThreeVector(-detX*2-SiBulkX*2-absThickness5,0,0),
			G4ThreeVector(detX+absThickness5/2+3*mm,0,0),
			abs_logic5,
			"abs_physic5",
			//SiOverall_logic,
			logicWorld,
			false,
			0,
			true);
	}
*/
////////////////////////////////////////////////////////////////////////////////
//			Scintillator High Energy Xray veto		      //
////////////////////////////////////////////////////////////////////////////////

/*
	G4Box* scintVeto_solid = new G4Box("scintVeto_solid",
				scintVetoThickness/2,
				scintY,
				scintZ);

	G4LogicalVolume* scintVeto_logic = new G4LogicalVolume(
				scintVeto_solid,
				matPlScin,
				"scintVeto_logic",
				0, 0, 0);

	G4PVPlacement* scintVeto_physic = new G4PVPlacement(
			0,
			G4ThreeVector(-5*wrapX-5*mm-absThickness3-absThickness4-scintVetoThickness/2,0,0),
			scintVeto_logic,
			"scintVeto_physic",
			SiOverall_logic,
			false,
			0,
			true);
*/
//==========================================================================
    //---------------PMT Geometry---------------
    //==========================================================================
//////////////////// defining phCathSolid as PMT so we can parameterise it /////////////
///*
         G4Tubs* phCathSolid = new G4Tubs(
                         "photocath_tube",
                         0,
                         outerRadius_pmt,
        //               (airGapThickness+wrapThickness)/2,
                         height_pmt/2,
                         0*deg,
                         360*deg);

         G4LogicalVolume* phCathLog = new G4LogicalVolume(phCathSolid,
                                        matBiAlkali,
                                        "phCathLog");
         G4PVPlacement* phCathPhysLarge = new G4PVPlacement(
                         0,
                         G4ThreeVector(spacing,-spacing,shift-height_NaILarge-height_pmt/2),
                         phCathLog,
                         "phCathPhysLarge",
                         holder_logic,
//                       pmtLog,
                         false,
                         1,
                         true);
///*
	 G4PVPlacement* phCathPhysSmall1 = new G4PVPlacement(
                         0,
                         G4ThreeVector(-spacing,-spacing,shift+height_NaILarge-height_NaISmall-height_NaISmall-height_pmt/2),
                         phCathLog,
                         "phCathPhysSmall1",
                         holder_logic,
//                       pmtLog,
                         false,
                         2,
                         true);
         G4PVPlacement* phCathPhysSmall2 = new G4PVPlacement(
                         0,
                         G4ThreeVector(spacing,spacing,shift+height_NaILarge-height_NaISmall-height_NaISmall-height_pmt/2),
                         phCathLog,
                         "phCathPhysSmall2",
                         holder_logic,
//                       pmtLog,
                         false,
                         3,
                         true);
         G4PVPlacement* phCathPhysLYSO = new G4PVPlacement(
                         0,
                         G4ThreeVector(-spacing,spacing,shift+height_NaILarge-length_LYSOArray-length_LYSOArray-height_pmt/2),
                         phCathLog,
                         "phCathPhysLYSO",
                         holder_logic,
//                       pmtLog,
                         false,
                         4,
                         true);
//*/
	
////////// add pmt as daughter of photocathode (impacts visualization only)

         G4Tubs* pmtSolid = new G4Tubs(
                         "pmt_tube",
                         0,
                         outerRadius_pmt,
                         (height_pmt-height_cath)/2,
                         //(height_pmt-height_cath)/2,
                         0*deg,
                         360*deg);

         G4LogicalVolume* pmtLog = new G4LogicalVolume(
                         pmtSolid,
                         //matBorGlass, //actually made of matBorGlass, but the material isn't a significant source
                         worldMaterial, //so, going with air (world material) insteadhttps://www.chem.uci.edu/~unicorn/243/handouts/pmt.pdf
                        "pmtLog");
///*
                        //there's a bug involving surface overlaps of daughter volumes for sensitive detectors in Geant4, so I'm just going to have the PMT overhang the edge a bit so it doesn't affect the sensitive detector performance of the photocathode volume. Doesn't matter because it's just a cosmetic thing anyways
        G4PVPlacement* pmtPhys = new G4PVPlacement(
                         0,
                         G4ThreeVector(0,0,-(height_cath+10*mm)/2),
                         pmtLog,
                         "pmtPhys",
                         phCathLog,
                         false,
                         0,
                         true);
//*/




    //==========================================================================
    //---------------Scintillator Sensitive Detector---------------
    //==========================================================================

/*
	G4SDManager* SDman = G4SDManager::GetSDMpointer();
	G4String SDnameC = "BeamRad/ScintiSensitiveDetector";
	mqScintSD* scintSD = new mqScintSD(SDnameC);
	SDman->AddNewDetector ( scintSD );
	NaISmall_logic->SetSensitiveDetector(scintSD);
	NaILarge_logic->SetSensitiveDetector(scintSD);
	LYSO_logic->SetSensitiveDetector(scintSD);
	SiDet_logic2->SetSensitiveDetector(scintSD);
	SiDet_logic3->SetSensitiveDetector(scintSD);
	SiDet_logic4->SetSensitiveDetector(scintSD);
	SiDet_logic5->SetSensitiveDetector(scintSD);
	scintVeto_logic->SetSensitiveDetector(scintSD);
	//abs_logic->SetSensitiveDetector(scintSD);
*/
   G4String SDnameOfScintDetector = "Scint_SD";
   mqScintSD* myScintSD = new mqScintSD(SDnameOfScintDetector);
   G4SDManager::GetSDMpointer()->AddNewDetector(myScintSD);


////////////////    G4 Vis attributes        /////////////////////////////
	
	G4VisAttributes* visAttribSiDet = new G4VisAttributes(G4Colour::White());
	visAttribSiDet->SetVisibility(true);
	
	G4VisAttributes* visAttribAbs = new G4VisAttributes(
			G4Colour::Gray());
	visAttribAbs->SetVisibility(true);

	G4VisAttributes* visAttribAbsAl = new G4VisAttributes(
			G4Colour::Blue());
	visAttribAbsAl->SetVisibility(true);

	G4VisAttributes* visAttribAbsSource = new G4VisAttributes(
			//G4Colour::Gray());
			G4Colour::Yellow());
	visAttribAbsSource->SetVisibility(true);
	//visAttribAbsSource->SetVisibility(false);

	G4VisAttributes* visAttribAbsCu = new G4VisAttributes(
			G4Colour::Red());
	visAttribAbsCu->SetVisibility(true);

	G4VisAttributes* visAttribAbsPb = new G4VisAttributes(
			G4Colour::Black());
	visAttribAbsPb->SetColor(0,0,0,0.3);
	visAttribAbsPb->SetVisibility(true);

	G4VisAttributes* visAttribAbsNi = new G4VisAttributes(
			G4Colour::Gray());
	visAttribAbsNi->SetVisibility(true);

	G4VisAttributes* visAttribAbsFe = new G4VisAttributes(
			G4Colour::Black());
	visAttribAbsFe->SetVisibility(true);
	
	G4VisAttributes* visAttribWrapExt = new G4VisAttributes(
			G4Colour::White());
	visAttribWrapExt->SetColour(0.5,0.5,0.5,0.1);
	visAttribWrapExt->SetVisibility(true);

/*
	G4VisAttributes* visAttribSiDetExt = new G4VisAttributes(G4Colour::Cyan());
	visAttribSiDetExt->SetVisibility(false);
	

	G4VisAttributes* visAttribLead = new G4VisAttributes(
			G4Colour::Black());
	visAttribLead->SetVisibility(true);
	
*/
	//SiLayer_logic->SetVisAttributes(visAttribSiDet);
	//SiBulk_logic->SetVisAttributes(visAttribAbsNi);
	case_logic->SetVisAttributes(visAttribWrapExt);
	holder_logic->SetVisAttributes(visAttribWrapExt);
//	holder_logic->SetVisAttributes(visAttWorld);
	AlCapSmall_logic->SetVisAttributes(visAttribAbsPb);
	AlCapLarge_logic->SetVisAttributes(visAttribAbsPb);
	AlSmall_logic->SetVisAttributes(visAttribAbsPb);
	AlLarge_logic->SetVisAttributes(visAttribAbsPb);
	NaISmall_logic->SetVisAttributes(visAttribSiDet);
	NaILarge_logic->SetVisAttributes(visAttribSiDet);
	LYSO_logic->SetVisAttributes(visAttribAbsPb);
	wrapNaISmall_logic->SetVisAttributes(visAttribWrapExt);
	wrapNaILarge_logic->SetVisAttributes(visAttribWrapExt);
	wrapLYSO_logic->SetVisAttributes(visAttribWrapExt);
	airgapNaISmall_logic->SetVisAttributes(visAttWorld);
	airgapNaILarge_logic->SetVisAttributes(visAttWorld);
	airgapLYSO_logic->SetVisAttributes(visAttWorld);
	Source_logic->SetVisAttributes(visAttribAbsSource);

/*
	abs_logic1->SetVisAttributes(visAttribAbsAl);
	abs_logic2->SetVisAttributes(visAttribAbsCu);
	abs_logic3->SetVisAttributes(visAttribAbsNi);
	abs_logic4->SetVisAttributes(visAttribAbsFe);
	SiDet_logic1->SetVisAttributes(visAttribSiDet);
	SiDet_logic2->SetVisAttributes(visAttribSiDet);
	SiDet_logic3->SetVisAttributes(visAttribSiDet);
	SiDet_logic4->SetVisAttributes(visAttribSiDet);
	SiDet_logic5->SetVisAttributes(visAttribSiDet);
	scintVeto_logic->SetVisAttributes(visAttribSiDet);

	cuRod_logic->SetVisAttributes(visAttribAbsCu);
	steelWall_logic->SetVisAttributes(visAttribAbsNi);
*/
////////////////////////////////////////////////////////////////////////////

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
   //reset at the begining of events
   //==========================================================================
   //---------------PMT Sensitive Detector---------------
   //==========================================================================
///*
	G4String SDnameOfPMTDetector = "PMT_SD";
   mqPMTSD* myPMTSD = new mqPMTSD(SDnameOfPMTDetector);

   G4SDManager::GetSDMpointer()->AddNewDetector(myPMTSD);
//	myPMTSD->InitPMTs(nbAllPmts);
	myPMTSD->SetR878_QE(GetPMTEff_R878());
//	myPMTSD->SetR7725_QE(GetPMTEff_R7725());
//	myPMTSD->SetET9814B_QE(GetPMTEff_ET9814B());
	phCathLog->SetSensitiveDetector(myPMTSD); //change HERE for PMT Scenario
//  	G4ThreeVector PMTPosition1 = G4ThreeVector(0,pmtYoffset,scintZ+height_pmt/2+(nLayers-1)*layerSpacing/2);
  //  	G4RotationMatrix rotm1  = G4RotationMatrix();//(G4ThreeVector(0, -1., 0. ), pi / 2.0);
//	G4Transform3D transform = G4Transform3D(rotm1,PMTPosition1);
//*/
/*
	G4PVPlacement* pmtPhys = new G4PVPlacement(transform,   //rotation,position
			                     pmtLog,                   //its logical volume
			                     "pmt_physic",                //its name
			                     logicWorld,              //its mother  volume
			                     false,                    //no boolean operation
			                     nbAllPmts,             //copy number
			                     true);           //checking overlaps
*/
///*
	// visual attributes for all PMTs;
	G4VisAttributes* visAttribPMT = new G4VisAttributes(G4Colour::Blue());
	visAttribPMT->SetForceWireframe(false);
	visAttribPMT->SetForceSolid(true);
	visAttribPMT->SetVisibility(true);

	//setting same color so it doesn't look bad in isometric view
	
	//G4VisAttributes* visAttribPhCath = new G4VisAttributes(G4Colour::Blue());
	G4VisAttributes* visAttribPhCath = new G4VisAttributes(G4Colour::Red());
	visAttribPhCath->SetForceWireframe(false);
	visAttribPhCath->SetForceSolid(false);
	visAttribPhCath->SetVisibility(true);

	pmtLog->SetVisAttributes(visAttribPMT);
	phCathLog->SetVisAttributes(visAttribPhCath);
//*/
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
	G4double photonEnergyWrap[nEntriesWrap]={ 0.1 * eV,10 * eV};
	G4double wrap_REFL[nEntriesWrap] = {wrapRefl,wrapRefl};//{0.95,0.95}
	G4double wrap_RIND[nEntriesWrap];

	G4MaterialPropertiesTable* mptWrap = new G4MaterialPropertiesTable();
	//mptWrap->AddProperty("TRANSMITTANCE",photonEnergyWrap, foil_REFL, nEntriesWrap);//->SetSpline(true);
	mptWrap->AddProperty("REFLECTIVITY",photonEnergyWrap, wrap_REFL, nEntriesWrap);
    
    opSWrapScintillator->SetMaterialPropertiesTable(mptWrap);

//define optical surfaces for each object in the sim, and match the optical properties of each surface to it

//    new G4LogicalSkinSurface("Wrap", wrap_logic, opSWrapScintillator);
    new G4LogicalSkinSurface("PhCath", phCathLog, opSDielectricBiAlkali);

    new G4LogicalSkinSurface("Wrap", wrapNaISmall_logic, opSWrapScintillator);
    new G4LogicalSkinSurface("Wrap", wrapNaILarge_logic, opSWrapScintillator);
    new G4LogicalSkinSurface("Wrap", wrapLYSO_logic, opSWrapScintillator);
//    new G4LogicalSkinSurface("PhCath", phCathLog, opSDielectricBiAlkali);

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
			//SiDet_physic, phCathPhys ,opSDielectricBiAlkali);

    new G4LogicalBorderSurface("PhCath->Scinti",
    			PMTParamPhys[0] ,barParamPhys[0] ,opSDielectricBiAlkali);
    			//phCathPhys ,SiDet_physic ,opSDielectricBiAlkali);
*/
/*
    new G4LogicalBorderSurface("AirGap->PhCath",
			airgap_physic, phCathPhys ,opSDielectricBiAlkali);

    new G4LogicalBorderSurface("PhCath->AirGap",
    			phCathPhys ,airgap_physic ,opSDielectricBiAlkali);
*/
//*/
//
//
    return physicWorld;

}

G4MaterialPropertiesTable* mqDetectorConstruction::SetOpticalPropertiesOfLYSO(){


G4MaterialPropertiesTable* mptPlScin = new G4MaterialPropertiesTable();

 const G4int nEntries= 23;//301;//100;

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
	ReadEJ200.open("/media/schmitz/Storage/computing/beamRadSimXRay/OpticalData/LYSOScintSpectrum.txt");
	if(ReadEJ200.is_open()){
	while(!ReadEJ200.eof()){
	ReadEJ200 >> pWavelength >> pSEff;
	pEnergy = (1240.0/pWavelength)*eV;
	photonEnergy[ScintEntry] = pEnergy;
	EJ200_SCINT[ScintEntry] = pSEff;
	G4cout << "read-in energy lyso: " << photonEnergy[ScintEntry] << " eff: " << EJ200_SCINT[ScintEntry] << G4endl;
	ScintEntry++;
	}
	}
	else
	G4cout << "Error opening file: " << "LYSOScintSpectrum.txt" << G4endl;
	ReadEJ200.close();


	for (int i = 0; i < nEntries; i++) {
		EJ200_RIND[i] = 1.81;//58; // refractive index at 425 nm
		//EJ200_ABSL[i] *= myPSAttenuationLength;
		EJ200_ABSL[i] = 0.5*m;//2.5 * m; // bulk attenuation at 425 nm
	}

	mptPlScin->AddProperty("FASTCOMPONENT", photonEnergy, EJ200_SCINT,
			nEntries);//->SetSpline(true);


	mptPlScin->AddProperty("ABSLENGTH", photonEnergy, EJ200_ABSL,
				nEntries);//->SetSpline(true);

	mptPlScin->AddConstProperty("SCINTILLATIONYIELD", 33000.*1.1 / MeV); //--- according to EJ200
	mptPlScin->AddConstProperty("RESOLUTIONSCALE", 1.0);
	mptPlScin->AddConstProperty("FASTTIMECONSTANT", 36 * ns); //decay time, according to EJ200
	mptPlScin->AddProperty("RINDEX", photonEnergy, EJ200_RIND, nEntries);//->SetSpline(true);

return mptPlScin;
}

G4MaterialPropertiesTable* mqDetectorConstruction::SetOpticalPropertiesOfNaI(){


G4MaterialPropertiesTable* mptPlScin = new G4MaterialPropertiesTable();

 const G4int nEntries= 25;//301;//100;

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
	ReadEJ200.open("/media/schmitz/Storage/computing/beamRadSimXRay/OpticalData/NaIScintSpectrum.txt");
	if(ReadEJ200.is_open()){
	while(!ReadEJ200.eof()){
	ReadEJ200 >> pWavelength >> pSEff;
	pEnergy = (1240.0/pWavelength)*eV;
	photonEnergy[ScintEntry] = pEnergy;
	EJ200_SCINT[ScintEntry] = pSEff;
	G4cout << "read-in energy nai: " << photonEnergy[ScintEntry] << " eff: " << EJ200_SCINT[ScintEntry] << G4endl;
	ScintEntry++;
	}
	}
	else
	G4cout << "Error opening file: " << "NaIScintSpectrum.txt" << G4endl;
	ReadEJ200.close();


	for (int i = 0; i < nEntries; i++) {
		EJ200_RIND[i] = 1.85;//58; // refractive index at 425 nm
		//EJ200_ABSL[i] *= myPSAttenuationLength;
		EJ200_ABSL[i] = 0.429*m;//2.5 * m; // bulk attenuation at 425 nm //from https://pdg.lbl.gov/2013/reviews/rpp2013-rev-particle-detectors-accel.pdf, table 32.4
	}

	mptPlScin->AddProperty("FASTCOMPONENT", photonEnergy, EJ200_SCINT,
			nEntries);//->SetSpline(true);


	mptPlScin->AddProperty("ABSLENGTH", photonEnergy, EJ200_ABSL,
				nEntries);//->SetSpline(true);

	mptPlScin->AddConstProperty("SCINTILLATIONYIELD", 38000.*1.1 / MeV); //--- according to EJ200
	mptPlScin->AddConstProperty("RESOLUTIONSCALE", 1.0);
	mptPlScin->AddConstProperty("FASTTIMECONSTANT", 250 * ns); //decay time, according to EJ200
	mptPlScin->AddProperty("RINDEX", photonEnergy, EJ200_RIND, nEntries);//->SetSpline(true);

return mptPlScin;
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
	pEnergy = (1240.0/pWavelength)*eV;
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
//	pQEff=1;
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
	// This is the main scaling variable for the PMTs in the sim!
        effVec.ScaleVector(1,1); //increase/decrease QE here if you want to scale to match experiment
        // 
        ReadPMTQEff.close();
	return effVec;
}

G4PhysicsVector mqDetectorConstruction::GetPMTEff_R7725(){
	
	std::ifstream ReadPMTQEff;
        ReadPMTQEff.open(mqOpticalFilePath+"PMT_R7725_QE.txt");
	G4PhysicsVector effVec;
	effVec.Retrieve(ReadPMTQEff,true);
	if (effVec.GetVectorLength()!=0) G4cout << "Quantum Efficiency successfully retrieved for PMT R7725" << G4endl;
	else G4cout << "ERROR: Vector length is zero!" << G4endl;
        effVec.ScaleVector(1,1); //increase QE here if you want to scale to match experiment
	ReadPMTQEff.close();
        return effVec;
}

G4PhysicsVector mqDetectorConstruction::GetPMTEff_ET9814B(){
	
	std::ifstream ReadPMTQEff;
        ReadPMTQEff.open(mqOpticalFilePath+"PMT_ET9814B_QE.txt");
	G4PhysicsVector effVec;
	effVec.Retrieve(ReadPMTQEff,true);
	if (effVec.GetVectorLength()!=0) G4cout << "Quantum Efficiency successfully retrieved for PMT ET9814B" << G4endl;
	else G4cout << "ERROR: Vector length is zero!" << G4endl;
        effVec.ScaleVector(1,1); //increase QE here if you want to scale to match experiment
        ReadPMTQEff.close();
	return effVec;
}

void mqDetectorConstruction::UpdateGeometry() {
	if (verbose >= 0) {
		G4cout << "BeamRad> Update geometry." << G4endl;
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



