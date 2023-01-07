#include <stdio.h>

#include "memory_handling.h"

int Something ( ) {
    HANDLE hProcess = FindProcessByExecutable("C:\\Program Files (x86)\\Steam\\steamapps\\common\\Cultist Simulator\\cultistsimulator.exe");
    DWORD rootdomainpointer = ReadRootMonoDomain32(hProcess);

    DWORD assembly = MonoDomain32GetAssemblyByName(hProcess, rootdomainpointer, "SecretHistories.Main");

    DWORD assemblyimage = MonoAssembly32GetImage(hProcess, assembly);

    DWORD watchman = MonoImage32GetClassByName(hProcess, assemblyimage, "SecretHistories.UI", "Watchman");

    DWORD watchmanstatic = MonoClass32GetStaticFieldData(hProcess, watchman);

    printf("Watchman static: %p\n", watchmanstatic);
    HexDump(hProcess, (LPVOID)watchmanstatic, 0x100);
}
/*
Watchman.Get<Compendium>().GetEntityById<Legacy>(legacyId);
Token token = Watchman.Get<HornedAxe>().GetDefaultSphereForUnknownToken().GetTokensWhere((Token t) => !t.Defunct).FirstOrDefault<Token>();
foreach (KeyValuePair<string, TokenItinerary> keyValuePair in Watchman.Get<Xamanek>().GetCurrentItinerariesForPath(base.Sphere.GetAbsolutePath()))
Vector2 gridCellSize = Watchman.Get<Meniscate>().GetGridCellSize();
return Watchman.Get<DealersTable>();

*/
