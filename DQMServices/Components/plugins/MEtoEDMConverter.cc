
/** \file MEtoEDMConverter.cc
 *  
 *  See header file for description of class
 *
 *  $Date: 2008/02/21 03:26:48 $
 *  $Revision: 1.4 $
 *  \author M. Strang SUNY-Buffalo
 */

#include "DQMServices/Components/plugins/MEtoEDMConverter.h"
#include "classlib/utils/StringList.h"
#include "classlib/utils/StringOps.h"

using namespace lat;

MEtoEDMConverter::MEtoEDMConverter(const edm::ParameterSet & iPSet) :
  fName(""), verbosity(0), frequency(0)
{
  std::string MsgLoggerCat = "MEtoEDMConverter_MEtoEDMConverter";

  // get information from parameter set
  fName = iPSet.getUntrackedParameter<std::string>("Name");
  verbosity = iPSet.getUntrackedParameter<int>("Verbosity");
  frequency = iPSet.getUntrackedParameter<int>("Frequency");
  
  // use value of first digit to determine default output level (inclusive)
  // 0 is none, 1 is basic, 2 is fill output, 3 is gather output
  verbosity %= 10;
  
  // print out Parameter Set information being used
  if (verbosity >= 0) {
    edm::LogInfo(MsgLoggerCat) 
      << "\n===============================\n"
      << "Initialized as EDProducer with parameter values:\n"
      << "    Name          = " << fName << "\n"
      << "    Verbosity     = " << verbosity << "\n"
      << "    Frequency     = " << frequency << "\n"
      << "===============================\n";
  }

  // get dqm info
  dbe = 0;
  dbe = edm::Service<DQMStore>().operator->();
  if (dbe) {
    if (verbosity) {
      dbe->setVerbose(1);
    } else {
      dbe->setVerbose(0);
    }
  }

  // information flags
  std::map<std::string,int> packages; // keep track just of package names
  unsigned nTH1F = 0; // count various objects we have
  unsigned nTH2F = 0;
  unsigned nTH3F = 0;
  unsigned nTProfile = 0;
  unsigned nTProfile2D = 0;
  unsigned nFloat = 0;
  unsigned nInt = 0;
  unsigned nString = 0;

  // get contents out of DQM
  std::vector<MonitorElement *>::iterator mmi, mme;
  std::vector<MonitorElement *> items(dbe->getAllContents(""));
  for (mmi = items.begin (), mme = items.end (); mmi != mme; ++mmi) {
    // keep track of leading directory (i.e. package)
    StringList dir = StringOps::split((*mmi)->getPathname(),"/");
    ++packages[dir[0]];

    // check type
    if (verbosity > 1) std::cout << "MEobject:" << std::endl;
    MonitorElement *me = *mmi;
    TObject *tobj = me->getRootObject();
    switch (me->kind())
    {
    case MonitorElement::DQM_KIND_INT:
      ++nInt;
      if (verbosity > 1)
	std::cout << "   scalar: " << tobj->GetName() << ": Int\n";
      break;

    case MonitorElement::DQM_KIND_REAL:
      ++nFloat;
      if (verbosity > 1)
	std::cout << "   scalar: " << tobj->GetName() << ": Float\n";
      break;

    case MonitorElement::DQM_KIND_STRING:
      ++nString;
      if (verbosity > 1)
	std::cout << "   scalar: " << tobj->GetName() << ": String\n";
      break;

    case MonitorElement::DQM_KIND_TH1F:
      ++nTH1F;
      if (verbosity > 1)
	std::cout << "   normal: " << tobj->GetName() << ": TH1F\n";
      break;

    case MonitorElement::DQM_KIND_TH2F:
      ++nTH2F;
      if (verbosity > 1)
	std::cout << "   normal: " << tobj->GetName() << ": TH2F\n";
      break;

    case MonitorElement::DQM_KIND_TH3F:
      ++nTH3F;
      if (verbosity > 1)
	std::cout << "   normal: " << tobj->GetName() << ": TH3F\n";
      break;

    case MonitorElement::DQM_KIND_TPROFILE:
      ++nTProfile;
      if (verbosity > 1)
	std::cout << "   normal: " << tobj->GetName() << ": TProfile\n";
      break;

    case MonitorElement::DQM_KIND_TPROFILE2D:
      ++nTProfile2D;
      if (verbosity > 1)
	std::cout << "   normal: " << tobj->GetName() << ": TProfile2D\n";
      break;

    default:
      edm::LogError(MsgLoggerCat)
	<< "ERROR: The DQM object '" << me->getFullname()
	<< "' is neither a ROOT object nor a recognised "
	<< "simple object.\n";
      continue;
    }
  } // end loop through monitor elements

  if (verbosity) {
    // list unique packages
    std::cout << "Packages accessing DQM:" << std::endl;
    std::map<std::string,int>::iterator pkgIter;
    for (pkgIter = packages.begin(); pkgIter != packages.end(); ++pkgIter) 
      std::cout << "  " << pkgIter->first << ": " << pkgIter->second 
		<< std::endl;

    std::cout << "We have " << nTH1F << " TH1F objects" << std::endl;
    std::cout << "We have " << nTH2F << " TH2F objects" << std::endl;
    std::cout << "We have " << nTH3F << " TH3F objects" << std::endl;
    std::cout << "We have " << nTProfile << " TProfile objects" << std::endl;
    std::cout << "We have " << nTProfile2D << " TProfile2D objects" 
	      << std::endl;
    std::cout << "We have " << nFloat << " Float objects" << std::endl;
    std::cout << "We have " << nInt << " Int objects" << std::endl;
    std::cout << "We have " << nString << " String objects" << std::endl;
  }
      
  // create persistent objects
  if (nTH1F)
    produces<MEtoEDM<TH1F>, edm::InRun>(fName);
  if (nTH2F)
    produces<MEtoEDM<TH2F>, edm::InRun>(fName);
  if (nTH3F)
    produces<MEtoEDM<TH3F>, edm::InRun>(fName);
  if (nTProfile)
    produces<MEtoEDM<TProfile>, edm::InRun>(fName);
  if (nTProfile2D)
    produces<MEtoEDM<TProfile2D>, edm::InRun>(fName);
  if (nFloat)
    produces<MEtoEDM<double>, edm::InRun>(fName);
  if (nInt)
    produces<MEtoEDM<int>, edm::InRun>(fName);
  if (nString)
    produces<MEtoEDM<TString>, edm::InRun>(fName);
} // end constructor

MEtoEDMConverter::~MEtoEDMConverter() 
{
} // end destructor

void
MEtoEDMConverter::beginJob(const edm::EventSetup& iSetup)
{
}

void
MEtoEDMConverter::endJob(void)
{
  std::string MsgLoggerCat = "MEtoEDMConverter_endJob";
  if (verbosity >= 0)
    edm::LogInfo(MsgLoggerCat) 
      << "Terminating having processed " << count.size() << " runs.";
  return;
}

void
MEtoEDMConverter::beginRun(edm::Run& iRun, const edm::EventSetup& iSetup)
{
  std::string MsgLoggerCat = "MEtoEDMConverter_beginRun";
    
  int nrun = iRun.run();
  
  // keep track of number of runs processed
  ++count[nrun];

  if (verbosity) {  // keep track of number of runs processed
    edm::LogInfo(MsgLoggerCat)
      << "Processing run " << nrun << " (" << count.size() << " runs total)";
  } else if (verbosity == 0) {
    if (nrun%frequency == 0 || count.size() == 1) {
      edm::LogInfo(MsgLoggerCat)
	<< "Processing run " << nrun << " (" << count.size() << " runs total)";
    }
  }
}

void
MEtoEDMConverter::endRun(edm::Run& iRun, const edm::EventSetup& iSetup)
{
  mestorage<TH1F> TH1FME;
  mestorage<TH2F> TH2FME;
  mestorage<TH3F> TH3FME;
  mestorage<TProfile> TProfileME;
  mestorage<TProfile2D> TProfile2DME;
  mestorage<double> FloatME;
  mestorage<int> IntME;
  mestorage<TString> StringME;

  std::string MsgLoggerCat = "MEtoEDMConverter_endRun";
  
  if (verbosity)
    edm::LogInfo (MsgLoggerCat) << "\nStoring MEtoEDM dataformat histograms.";

  // extract ME information into vectors
  std::vector<MonitorElement *>::iterator mmi, mme;
  std::vector<MonitorElement *> items(dbe->getAllContents(""));
  for (mmi = items.begin (), mme = items.end (); mmi != mme; ++mmi) {
    MonitorElement *me = *mmi;
    TObject *tobj = me->getRootObject();

    // already extracted full path name in constructor
    if (verbosity > 1) std::cout << "name: " << me->getFullname() << std::endl;

    // get monitor elements
    if (verbosity > 1) std::cout << "MEobject:" << std::endl;
    switch (me->kind())
    {
    case MonitorElement::DQM_KIND_INT:
      if (verbosity > 1)
	std::cout << "   scalar: " << tobj->GetName() << ": Int\n";
      if (verbosity > 1)
	std::cout << "      value: " << me->getIntValue() << std::endl;
      IntME.object.push_back(me->getIntValue());
      IntME.name.push_back(me->getFullname());
      IntME.tags.push_back(me->getTags());
      break;

    case MonitorElement::DQM_KIND_REAL:
      if (verbosity > 1)
	std::cout << "   scalar: " << tobj->GetName() << ": Float\n";
      if (verbosity > 1)
	std::cout << "      value: " << me->getFloatValue() << std::endl;
      FloatME.object.push_back(me->getFloatValue());
      FloatME.name.push_back(me->getFullname());
      FloatME.tags.push_back(me->getTags());
      break;

    case MonitorElement::DQM_KIND_STRING:
      if (verbosity > 1)
	std::cout << "   scalar: " << tobj->GetName() << ": String\n";
      if (verbosity > 1)
	std::cout << "      value: " << me->getStringValue() << std::endl;
      StringME.object.push_back(me->getStringValue());
      StringME.name.push_back(me->getFullname());
      StringME.tags.push_back(me->getTags());
      break;

    case MonitorElement::DQM_KIND_TH1F:
      if (verbosity > 1)
	std::cout << "   normal: " << tobj->GetName() << ": TH1F\n";
      TH1FME.object.push_back(*me->getTH1F());
      TH1FME.name.push_back(me->getFullname());
      TH1FME.tags.push_back(me->getTags());
      me->Reset();
      break;

    case MonitorElement::DQM_KIND_TH2F:
      if (verbosity > 1)
	std::cout << "   normal: " << tobj->GetName() << ": TH2F\n";
      TH2FME.object.push_back(*me->getTH2F());
      TH2FME.name.push_back(me->getFullname());
      TH2FME.tags.push_back(me->getTags());
      me->Reset();
      break;

    case MonitorElement::DQM_KIND_TH3F:
      if (verbosity > 1)
	std::cout << "   normal: " << tobj->GetName() << ": TH3F\n";
      TH3FME.object.push_back(*me->getTH3F());
      TH3FME.name.push_back(me->getFullname());
      TH3FME.tags.push_back(me->getTags());
      me->Reset();
      break;

    case MonitorElement::DQM_KIND_TPROFILE:
      if (verbosity > 1)
	std::cout << "   normal: " << tobj->GetName() << ": TProfile\n";
      TProfileME.object.push_back(*me->getTProfile());
      TProfileME.name.push_back(me->getFullname());
      TProfileME.tags.push_back(me->getTags());
      me->Reset();
      break;

    case MonitorElement::DQM_KIND_TPROFILE2D:
      if (verbosity > 1)
	std::cout << "   normal: " << tobj->GetName() << ": TProfile2D\n";
      TProfile2DME.object.push_back(*me->getTProfile2D());
      TProfile2DME.name.push_back(me->getFullname());
      TProfile2DME.tags.push_back(me->getTags());
      me->Reset();
      break;

    default:
      edm::LogError(MsgLoggerCat)
	<< "ERROR: The DQM object '" << me->getFullname()
	<< "' is neither a ROOT object nor a recognised "
	<< "simple object.\n";
      continue;
    }
  } // end loop through monitor elements

  // produce objects to put in events
  if (! TH1FME.object.empty()) {
    std::auto_ptr<MEtoEDM<TH1F> > pOut1(new MEtoEDM<TH1F>);
    pOut1->putMEtoEdmObject(TH1FME.name,TH1FME.tags,TH1FME.object);
    iRun.put(pOut1,fName);
  }
  if (! TH2FME.object.empty()) {
    std::auto_ptr<MEtoEDM<TH2F> > pOut2(new MEtoEDM<TH2F>);
    pOut2->putMEtoEdmObject(TH2FME.name,TH2FME.tags,TH2FME.object);
    iRun.put(pOut2,fName);
  }
  if (! TH3FME.object.empty()) {
    std::auto_ptr<MEtoEDM<TH3F> > pOut3(new MEtoEDM<TH3F>);
    pOut3->putMEtoEdmObject(TH3FME.name,TH3FME.tags,TH3FME.object);
    iRun.put(pOut3,fName);
  }
  if (! TProfileME.object.empty()) {
    std::auto_ptr<MEtoEDM<TProfile> > pOut4(new MEtoEDM<TProfile>);
    pOut4->putMEtoEdmObject(TProfileME.name,TProfileME.tags,TProfileME.object);
    iRun.put(pOut4,fName);
  }
  if (! TProfile2DME.object.empty()) {
    std::auto_ptr<MEtoEDM<TProfile2D> > pOut5(new MEtoEDM<TProfile2D>);
    pOut5->putMEtoEdmObject(TProfile2DME.name,TProfile2DME.tags, 
			    TProfile2DME.object);
    iRun.put(pOut5,fName);
  }
  if (! FloatME.object.empty()) {
    std::auto_ptr<MEtoEDM<double> > pOut6(new MEtoEDM<double>);
    pOut6->putMEtoEdmObject(FloatME.name,FloatME.tags,FloatME.object);
    iRun.put(pOut6,fName);
  }
  if (! IntME.object.empty()) {
    std::auto_ptr<MEtoEDM<int> > pOut7(new MEtoEDM<int>);
    pOut7->putMEtoEdmObject(IntME.name,IntME.tags,IntME.object);
    iRun.put(pOut7,fName);
  }
  if (! StringME.object.empty()) {
    std::auto_ptr<MEtoEDM<TString> > 
      pOut8(new MEtoEDM<TString>);
    pOut8->putMEtoEdmObject(StringME.name,StringME.tags,StringME.object);
    iRun.put(pOut8,fName);
  }
}

void
MEtoEDMConverter::produce(edm::Event& iEvent, const edm::EventSetup& iSetup)
{
}
