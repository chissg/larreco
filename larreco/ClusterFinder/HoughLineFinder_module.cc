////////////////////////////////////////////////////////////////////////
//
// \file HoughLineFinder_module.cc
//
// \author kinga.partyka@yale.edu
//
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
//
// \file HoughLineFinder.cxx
//
// \author joshua.spitz@yale.edu
//
//  This algorithm is designed to find lines (Houghclusters) from clusters found by DBSCAN 
//  after deconvolution and hit finding.
//  The algorithm is based on: 
//  Queisser, A. "Computing the Hough Transform", C/C++ Users Journal 21, 12 (Dec. 2003).
//  Niblack, W. and Petkovic, D. On Improving the Accuracy of the Hough Transform", Machine 
//  Vision and Applications 3, 87 (1990)  
////////////////////////////////////////////////////////////////////////

extern "C" {
#include <sys/types.h>
#include <sys/stat.h>
}
#include <sstream>
#include <fstream>
#include <math.h>
#include <algorithm>
#include <vector>
#include <vector>
#include <string>
#include <iomanip>

// ROOT includes
#include <TCanvas.h>
#include "TDatabasePDG.h"
#include "TSystem.h"
#include "TMath.h"

// ART includes
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h" 
#include "fhiclcpp/ParameterSet.h" 
#include "art/Framework/Principal/Handle.h" 
#include "art/Framework/Services/Registry/ServiceHandle.h" 
#include "art/Framework/Services/Optional/TFileService.h" 
#include "art/Framework/Services/Optional/TFileDirectory.h" 
#include "messagefacility/MessageLogger/MessageLogger.h" 
#include "CLHEP/Random/JamesRandom.h"

// art extensions
#include "nutools/RandomUtils/NuRandomService.h"

// LArSoft includes 
#include "lardataobj/RawData/RawDigit.h"
#include "lardataobj/RecoBase/Cluster.h"
#include "lardataobj/RecoBase/Hit.h"
#include "lardata/Utilities/AssociationUtil.h"
#include "larreco/RecoAlg/HoughBaseAlg.h"
#include "art/Framework/Core/EDProducer.h"

//#ifndef CLUSTER_HOUGHLINEFINDER_H
//#define CLUSTER_HOUGHLINEFINDER_H


class TH1F;
class TTree;

namespace cluster {
   
  class HoughLineFinder : public art::EDProducer {
    
  public:
    
    explicit HoughLineFinder(fhicl::ParameterSet const& pset); 

    void produce(art::Event& evt) override;
     
    
  private:

    std::string fDBScanModuleLabel;    
    unsigned int fHoughSeed;

    HoughBaseAlg fHLAlg;            ///< object that does the Hough Transform
  
  };
  
  
}

//#endif // CLUSTER_HOUGHLINEFINDER_H


namespace cluster {

  //------------------------------------------------------------------------------
  HoughLineFinder::HoughLineFinder(fhicl::ParameterSet const& pset) 
    : EDProducer{pset}
    , fDBScanModuleLabel{pset.get< std::string >("DBScanModuleLabel")}
    , fHoughSeed{pset.get< unsigned int >("HoughSeed", 0)}
    , fHLAlg(pset.get< fhicl::ParameterSet >("HoughBaseAlg"))
  {
    fHLAlg.reconfigure(pset.get< fhicl::ParameterSet >("HoughBaseAlg"));
    produces< std::vector<recob::Cluster> >();
    produces< art::Assns<recob::Cluster, recob::Hit> >();
    
    // Create random number engine needed for PPHT;
    // obtain the random seed from NuRandomService,
    // unless overridden in configuration with key "Seed"
    // remember that HoughSeed will override this on each event if specified
    (void)art::ServiceHandle<rndm::NuRandomService>()
      ->createEngine(*this, pset, "Seed");
  }
  
  //------------------------------------------------------------------------------
  void HoughLineFinder::produce(art::Event& evt)
  {
  
    //////////////////////////////////////////////////////
    // here is how to get a collection of objects out of the file
    // and connect it to a art::Handle
    //////////////////////////////////////////////////////
    // Read in the clusterList object(s).
    art::Handle< std::vector<recob::Cluster> > clusterListHandle;
    evt.getByLabel(fDBScanModuleLabel,clusterListHandle);
  
    //art::PtrVector<recob::Cluster> clusIn;
    std::vector<art::Ptr<recob::Cluster> > clusIn;
    for(unsigned int ii = 0; ii < clusterListHandle->size(); ++ii){
      art::Ptr<recob::Cluster> cluster(clusterListHandle, ii);
      clusIn.push_back(cluster);
    }
    
    //Point to a collection of clusters to output.
    std::unique_ptr<std::vector<recob::Cluster> > ccol(new std::vector<recob::Cluster>);
    std::unique_ptr< art::Assns<recob::Cluster, recob::Hit> > assn(new art::Assns<recob::Cluster, recob::Hit>);
    
    // make a std::vector< art::PtrVector<recob::Hit> >
    // to hold the associated hits of the Hough Transform 
    std::vector< art::PtrVector<recob::Hit> > clusHitsOut;
    
    size_t numclus = 0;
     
    art::ServiceHandle<art::RandomNumberGenerator> rng;
    CLHEP::HepRandomEngine &engine = rng->getEngine(art::ScheduleID::first(),
                                                    moduleDescription().moduleLabel());
    // If a nonzero random number seed has been provided, 
    // overwrite the seed already initialized
    if(fHoughSeed != 0){
      engine.setSeed(fHoughSeed,0);
    } 

    numclus = fHLAlg.FastTransform(clusIn, *ccol, clusHitsOut, engine, evt, fDBScanModuleLabel);


    //size_t Transform(std::vector<art::Ptr<recob::Cluster> >           & clusIn,
                            //std::vector<recob::Cluster>                      & ccol,  
  		     //std::vector< art::PtrVector<recob::Hit> >      & clusHitsOut,
  		     //art::Event                                const& evt,
  		     //std::string                               const& label);
  
    MF_LOG_DEBUG("HoughLineClusters") << "found " << numclus << "clusters with HoughBaseAlg";
  
  
    mf::LogVerbatim("Summary") << std::setfill('-') << std::setw(175) << "-" << std::setfill(' ');
    mf::LogVerbatim("Summary") << "HoughLineFinder Summary:";
    for(size_t i = 0; i < ccol->size(); ++i){
      mf::LogVerbatim("Summary") << ccol->at(i);
  
      // associat the hits to this cluster
      util::CreateAssn(*this, evt, *(ccol.get()), clusHitsOut[i], *(assn.get()), i);
    }
  
    evt.put(std::move(ccol));
    evt.put(std::move(assn));
    return;
  }
  
  
  



} // end namespace

namespace cluster{

  DEFINE_ART_MODULE(HoughLineFinder)
  
} 
