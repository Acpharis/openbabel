#include "obtest.h"
#include <openbabel/mol.h>
#include <openbabel/obconversion.h>

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

using namespace std;
using namespace OpenBabel;

static bool mdoMultiMoleculeFile(const std::string &filename)
{
  std::ifstream ifs;
  ifs.open(filename.c_str());
  OB_REQUIRE( ifs );

  OBMol mol;
  OBConversion conv(&ifs, &cout);
  OBFormat *format = conv.FormatFromExt(filename.c_str());
  OBFormat *canSMI = conv.FindFormat("can");
  OBFormat *mol2f  = conv.FindFormat("mol2");
  OBFormat *smi    = conv.FindFormat("smi");
  OB_REQUIRE(format);
  OB_REQUIRE(canSMI);
  OB_REQUIRE(mol2f);
  OB_REQUIRE(smi);

  OB_REQUIRE(conv.SetInAndOutFormats(format, canSMI)); 

  string output, mol2str, roundtrip; // first output string, intermediate mol2, then the roundtrip
  OBMol round2; // result of reading first output as canonical SMILES
  OBMol round3; // result of reading second output as mol2
  OBConversion conv2; // duplicate to prevent having to constantly change formats
  OB_REQUIRE(conv2.SetInAndOutFormats(canSMI, mol2f));
  OBConversion conv3; // duplicate to prevent having to constantly change formats
  OB_REQUIRE(conv3.SetInAndOutFormats(mol2f, canSMI));

  bool result = true;
  conv.SetInStream(&ifs);
  int testCount = 0;
  int failed = 0;

  while (1) {
    if (!conv.Read(&mol)) {
      // failed read, try again
      if (!conv.Read(&mol))
        break; // we tried twice, so break
    }
    testCount++;
    if (testCount % 1000 == 0)
      cout << testCount << " completed" << endl;

    mol.SetTitle("");
    output = conv.WriteString(&mol, true); // trim whitespace

    OB_REQUIRE(conv2.ReadString(&round2, output));
    round2.SetTitle("");
    mol2str = conv2.WriteString(&round2, true);

    OB_REQUIRE(conv3.ReadString(&round3, mol2str));
    round3.SetTitle("");
    roundtrip = conv3.WriteString(&round2, true);
    if (roundtrip != output) {
      failed++;
      result = false;
      if (strcasecmp(output.c_str(), roundtrip.c_str()) != 0)
        cout << "Failed roundtrip: \n  " << output << " -> \n  " << roundtrip << "\n";
      else
        cout << "Failed aromaticity: \n " << output << "\n";
    }
  }
  cout << "PASSED TESTS: " << testCount - failed << "/" << testCount << endl;
  return result;
}


int canonmol2consistenttest(int argc, char* argv[])
{
  int defaultchoice = 1;
  int choice = defaultchoice;

  if (argc > 1) {
    if(sscanf(argv[1], "%d", &choice) != 1) {
      printf("Couldn't parse that input as a number\n");
      return -1;
    }
  }

  // Define location of file formats for testing
#ifdef FORMATDIR
  char env[BUFF_SIZE];
  snprintf(env, BUFF_SIZE, "BABEL_LIBDIR=%s", FORMATDIR);
  putenv(env);
#endif

  switch(choice) {
    case 1:
      OB_ASSERT( mdoMultiMoleculeFile(OBTestUtil::GetFilename("forcefield.sdf")) );
      break;
    case 2:
      OB_ASSERT( mdoMultiMoleculeFile(OBTestUtil::GetFilename("filterset.sdf")) );
      break;
    case 3:
      OB_ASSERT( mdoMultiMoleculeFile(OBTestUtil::GetFilename("cantest.sdf")) );
      break;
    case 4:
      OB_ASSERT( mdoMultiMoleculeFile(OBTestUtil::GetFilename("VEHICLe.smi")) );
      break;
    default:
      cout << "Test numer " << choice << " does not exist!\n";
      return -1;
  }
//  OB_ASSERT( doMultiMoleculeFile(GetFilename("cansmi-roundtrip.smi")) );

  return 0;
}

