cd ..\..\..

echo on
if not exist build\include\cantera\kernel mkdir build\include\cantera\kernel

cd Cantera\src\base

copy Array.h        ..\..\..\build\include\cantera\kernel
copy FactoryBase.h  ..\..\..\build\include\cantera\kernel
copy LogPrintCtrl.h ..\..\..\build\include\cantera\kernel
copy PrintCtrl.h    ..\..\..\build\include\cantera\kernel
copy XML_Writer.h   ..\..\..\build\include\cantera\kernel
copy clockWC.h      ..\..\..\build\include\cantera\kernel
copy config.h       ..\..\..\build\include\cantera\kernel
copy ct_defs.h      ..\..\..\build\include\cantera\kernel
copy ctexceptions.h ..\..\..\build\include\cantera\kernel
copy ctml.h         ..\..\..\build\include\cantera\kernel
copy global.h       ..\..\..\build\include\cantera\kernel
copy logger.h       ..\..\..\build\include\cantera\kernel
copy mdp_allo.h     ..\..\..\build\include\cantera\kernel
copy plots.h        ..\..\..\build\include\cantera\kernel
copy stringUtils.h  ..\..\..\build\include\cantera\kernel
copy units.h        ..\..\..\build\include\cantera\kernel
copy utilities.h    ..\..\..\build\include\cantera\kernel
copy vec_functions.h  ..\..\..\build\include\cantera\kernel
copy xml.h          ..\..\..\build\include\cantera\kernel

cd ..\..\..\win32\vc9\base
echo off
echo 'ok' 
