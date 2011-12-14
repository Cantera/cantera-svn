"""
SCons build script for Cantera

Basic usage:
    'scons help' - print a description of user-specifiable options.

    'scons build' - Compile Cantera and the language interfaces using
                    default options.

    '[sudo] scons install' - Install Cantera.

    'scons test' - Run regression test suite

    'scons test-clean' - Delete files created while running the
                         regression tests.
"""

from buildutils import *
import subst
import platform, sys, os

if not COMMAND_LINE_TARGETS:
    # Print usage help
    print __doc__
    sys.exit(0)

env = Environment(tools = ['default', 'textfile'])
env.AddMethod(RecursiveInstall)
subst.TOOL_SUBST(env)
add_RegressionTest(env)

# ******************************************************
# *** Set system-dependent defaults for some options ***
# ******************************************************

class defaults: pass
if os.name == 'posix':
    defaults.prefix = '/usr/local'
    defaults.boostIncDir = '/usr/include'
    defaults.boostLibDir = '/usr/lib'
elif os.name == 'nt':
    defaults.prefix = os.environ['ProgramFiles']
    defaults.boostIncDir = ''
    defaults.boostLibDir = ''
else:
    print "Error: Unrecognized operating system '%s'" % os.name
    sys.exit(1)

if env['CC'] == 'gcc':
    defaults.cxxFlags = '-O3 -Wall'
    defaults.fPIC = ['-fPIC']
elif env['CC'] == 'cl': # Visual Studio
    defaults.cxxFlags = '/EHsc'
    defaults.fPIC = []
else:
    print "Error: Unrecognized C compiler '%s'" % env['CC']

# **************************************
# *** Read user-configurable options ***
# **************************************

opts = Variables('cantera.conf')
opts.AddVariables(
    PathVariable(
        'prefix',
        'Set this to the directory where Cantera should be installed.',
        defaults.prefix, PathVariable.PathIsDirCreate),
    EnumVariable(
        'python_package',
        """If you plan to work in Python, or you want to use the
           graphical MixMaster application, then you need the 'full'
           Cantera Python Package. If, on the other hand, you will
           only use Cantera from some other language (e.g. MATLAB or
           Fortran 90/95) and only need Python to process .cti files,
           then you only need a 'minimal' subset of the package
           (actually, only one file). The default behavior is to build
           the Python package if the required prerequsites (numpy) are
           installed.""",
        'default', ('full', 'minimal', 'none','default')),
    PathVariable(
        'python_cmd',
        """Cantera needs to know where to find the Python
           interpreter. If PYTHON_CMD is not set, then the
           configuration process will use the same Python interpreter
           being used by SCons.""",
        sys.executable),
    EnumVariable(
        'python_array',
        """The Cantera Python interface requires one of the Python
           array packages listed. Support for the legacy 'numeric' and
           'numarray' packages is deprecated, and will be removed in a
           future version of Cantera.""",
        'numpy', ('numpy', 'numarray', 'numeric')),
    PathVariable(
        'python_array_home',
        """If numpy was installed using the --home option, set this to
           the home directory for numpy.""",
        '', PathVariable.PathAccept),
    PathVariable(
        'cantera_python_home',
        """If you want to install the Cantera Python package somewhere
           other than the default 'site-packages' directory within the
           Python library directory, then set this to the desired
           directory. This is useful when you do not have write access
           to the Python library directory.""",
        '', PathVariable.PathAccept),
    EnumVariable(
        'matlab_toolbox',
        """This variable controls whether the Matlab toolbox will be
           built. If it is set to 'default', the Matlab toolbox will
           be built if Matlab can be found in the $PATH. Note that you
           may need to run 'mex -setup' within Matlab to configure it
           for your C++ compiler before building Cantera.""",
        'default', ('y', 'n', 'default')),
    PathVariable(
        'matlab_cmd',
        'Path to the Matlab executable.',
        'matlab', PathVariable.PathAccept),
    EnumVariable(
        'f90_interface',
        """This variable controls whether the Fortran 90/95 interface
           will be built. If set to 'default', the builder will look
           for a compatible Fortran compiler in the $PATH, and compile
           the Fortran 90 interface if one is found.""",
        'default', ('y', 'n', 'default')),
    PathVariable(
        'F90',
        """The Fortran 90 compiler. If unspecified, the builder will
           look for a compatible compiler (gfortran, ifort, g95) in
           the $PATH.""",
        '', PathVariable.PathAccept),
    ('F90FLAGS',
     'Compilation options for the Fortran 90 compiler.',
     '-O3'),
    BoolVariable(
        'debug',
        """Enable extra printing code to aid in debugging.""",
        False),
    BoolVariable(
        'with_lattice_solid',
        """Include thermodynamic model for lattice solids in the
           Cantera kernel.""",
        True),
    BoolVariable(
        'with_metal',
        """Include thermodynamic model for metals in the Cantera kernel.""",
        True),
    BoolVariable(
        'with_stoich_substance',
        """Include thermodynamic model for stoichiometric substances
           in the Cantera kernel.""",
        True),
    BoolVariable(
        'with_semiconductor',
        """Include thermodynamic model for semiconductors in the Cantera kernel.""",
        True),
    BoolVariable(
        'with_adsorbate',
        """Include thermodynamic model for adsorbates in the Cantera kernel""",
        True),
    BoolVariable(
        'with_spectra',
        """Include spectroscopy capability in the Cantera kernel.""",
        True),
    BoolVariable(
        'with_pure_fluids',
        """Include accurate liquid/vapor equations of state for
           several fluids, including water, nitrogen, hydrogen,
           oxygen, methane, and HFC-134a.""",
        True),
    BoolVariable(
        'with_ideal_solutions',
        """Include capabilities for working with ideal solutions.""",
        True),
    BoolVariable(
        'with_electrolytes',
        """Enable expanded electrochemistry capabilities, including
           thermodynamic models for electrolyte solutions.""",
        True),
    BoolVariable(
        'with_prime',
        """Enable generating phase models from PrIMe models. For more
           information about PrIME, see http://www.primekinetics.org 
           WARNING: Support for PrIMe is experimental!""",
        False),
    BoolVariable(
        'with_h298modify_capability',
        """Enable changing the 298K heats of formation directly via
           the C++ layer.""",
        False),
    BoolVariable(
        'enable_ck',
        """Build the ck2cti program that converts Chemkin input files
           to Cantera format (.cti). If you don't use Chemkin format
           files, or if you run ck2cti on some other machine, you can
           set this to 'n'.""",
        True),
    BoolVariable(
        'with_kinetics',
        """Enable homogeneous kinetics.""",
        True),
    BoolVariable(
        'with_hetero_kinetics',
        """Enable heterogeneous kinetics (surface chemistry). This
           also enables charge transfer reactions for
           electrochemistry.""",
        True),
    BoolVariable(
        'with_reaction_paths',
        """Enable reaction path analysis""",
        True),
    BoolVariable(
        'with_vcsnonideal',
        """Enable vcs equilibrium package for nonideal phases""",
        True),
    BoolVariable(
        'enable_transport',
        'Enable transport property calculations.',
        True),
    BoolVariable(
        'enable_equil',
        'Enable chemical equilibrium calculations',
        True),
    BoolVariable(
        'enable_reactors',
        'Enable stirred reactor models',
        True),
    BoolVariable(
        'enable_flow1d',
        'Enable one-dimensional flow models',
        True),
    BoolVariable(
        'enable_solvers',
        'Enable ODE integrators and DAE solvers',
        True),
    BoolVariable(
        'with_html_log_files',
        """write HTML log files. Some multiphase equilibrium
           procedures can write copious diagnostic log messages. Set
           this to 'n' to disable this capability. (results in
           slightly faster equilibrium  calculations)""",
        True),
    EnumVariable(
        'use_sundials',
        """Cantera uses the CVODE or CVODES ODE integrator to
           time-integrate reactor network ODE's and for various other
           purposes. An older version of CVODE comes with Cantera, but
           it is possible to use the latest version as well, which now
           supports sensitivity analysis (CVODES). CVODES is a part of
           the 'sundials' package from Lawrence Livermore National
           Laboratory. Sundials is not distributed with Cantera, but
           it is free software that may be downloaded and installed
           separately. If you leave USE_SUNDIALS = 'default', then it
           will be used if you have it, and if not the older CVODE
           will be used. Or set USE_SUNDIALS to 'y' or 'n' to force
           using it or not. Note that sensitivity analysis with
           Cantera requires use of sundials. See:
           http://www.llnl.gov/CASC/sundials""",
        'default', ('default', 'y', 'n')),
    EnumVariable(
        'sundials_version',
        """It is recommended that you install the newest release of
           sundials (currently 2.4.0) before building Cantera. But if
           you want to use an older version, set SUNDIALS_VERSION to
           the version you have.""",
        '2.4', ('2.2','2.3','2.4')),
    PathVariable(
        'sundials_include',
        """The directory where the Sundials header files are
           installed. This should be the directory that contains the
           "cvodes", "nvector", etc. subdirectories. Not needed if the
           headers are installed in a standard location,
           e.g. /usr/include.""",
        '', PathVariable.PathAccept),
    PathVariable(
        'sundials_libdir',
        """The directory where the sundials static libraries are
           installed.  Not needed if the libraries are installed in a
           standard location, e.g. /usr/lib.""",
        '', PathVariable.PathAccept),
    ('blas_lapack_libs',
     """Cantera comes with Fortran (or C) versions of those parts of
        BLAS and LAPACK it requires. But performance may be better if
        you use a version of these libraries optimized for your
        machine hardware. If you want to use your own libraries, set
        blas_lapack_libs to the the list of libraries that should be
        passed to the linker, separated by commas, e.g. "lapack,blas"
        or "lapack,f77blas,cblas,atlas". """,
     ''),
    PathVariable('blas_lapack_dir',
     """Directory containing the libraries specified by 'blas_lapack_libs'.""",
     '', PathVariable.PathAccept),
    EnumVariable(
        'lapack_names',
        """Set depending on whether the procedure names in the
           specified libraries are lowercase or uppercase. If you
           don't know, run 'nm' on the library file (e.g. 'nm
           libblas.a').""",
        'lower', ('lower','upper')),
    BoolVariable(
        'lapack_ftn_trailing_underscore', '', True),
    BoolVariable(
        'lapack_ftn_string_len_at_end', '', True),
    ('CXX',
     'The C++ compiler to use.',
     env['CXX']),
    ('CC',
     """The C compiler to use. This is only used to compile CVODE and
        the Python extension module.""",
     env['CC']),
    ('CXXFLAGS',
     'C++ Compiler flags.',
     defaults.cxxFlags),
    BoolVariable(
        'build_thread_safe',
        """Cantera can be built so that it is thread safe. Doing so
           requires using procedures from the Boost library, so if you
           want thread safety then you need to get and install Boost
           (http://www.boost.org) if you don't have it.  This is
           turned off by default, in which case Boost is not required
           to build Cantera.""",
        False),
    PathVariable(
        'boost_inc_dir',
        'Location of the Boost header files',
        defaults.boostIncDir, PathVariable.PathAccept),
    PathVariable(
        'boost_lib_dir',
        'Directory containing the Boost.Thread library',
        defaults.boostLibDir, PathVariable.PathAccept),
    ('boost_thread_lib',
     'The name of the Boost.Thread library.',
     'boost_thread'),
    BoolVariable(
        'build_with_f2c',
        """For external procedures written in Fortran 77, both the
           original F77 source code and C souce code generated by the
           'f2c' program are included.  Set this to "n" if you want to
           build Cantera using the F77 sources in the ext directory.""",
        True),
    ('F77',
     """Compiler used to build the external Fortran 77 procedures from
        the Fortran source code""",
     env['F77']),
    ('F77FLAGS',
     """Fortran 77 Compiler flags. Note that the Fortran compiler
      flags must be set to produce object code compatible with the
      C/C++ compiler you are using.""",
     '-O3'),
    PathVariable('graphvisdir',
     """The directory location of the graphviz program, dot. dot is
        used for creating the documentation, and for making reaction
        path diagrams. If "dot" is in your path, you can leave this
        unspecified. NOTE: Matlab comes with a stripped-down version
        of 'dot'. If 'dot' is on your path, make sure it is not the
        Matlab version!""",
     '', PathVariable.PathAccept),
    ('ct_shared_lib',
     '',
     'clib'),
    ('rpfont',
     """The font to use in reaction path diagrams. This must be a font
        name recognized by the 'dot' program. On linux systems, this
        should be lowercase 'helvetica'.""",
     'Helvetica'),
    ('cantera_version', '', '1.8.x')
    )

opts.Update(env)
opts.Save('cantera.conf', env)


if 'help' in COMMAND_LINE_TARGETS:
    ### Print help about configuration options and exit.
    print """
        **************************************************
        *   Configuration options for building Cantera   *
        **************************************************

The following options can be passed to SCons to customize the Cantera
build process. They should be given in the form:

    scons build option1=value1 option2=value2

Variables set in this way will be stored in the 'cantera.conf' file
and reused automatically on subsequent invocations of
scons. Alternatively, the configuration options can be entered
directly into 'cantera.conf' before running 'scons build'. The format
of this file is:

    option1 = 'value1'
    option2 = 'value2'

        **************************************************
"""

    for opt in opts.options:
        print '\n'.join(formatOption(env, opt))
    sys.exit(0)


# ********************************************
# *** Configure system-specific properties ***
# ********************************************
env['OS'] = platform.system()

# Try to find a Fortran compiler:
if env['f90_interface'] in ('y','default'):
    foundF90 = False
    if env['F90']:
        env['f90_interface'] = 'y'
        if which(env['F90']) is not None:
            foundF90 = True
        else:
            print "WARNING: Couldn't find specified Fortran compiler: '%s'" % env['F90']

    for compiler in ['gfortran', 'ifort', 'g95']:
        if foundF90:
            break
        if which(compiler) is not None:
            print "INFO: Using '%s' to build the Fortran 90 interface" % which(compiler)
            env['F90'] = compiler
            foundF90 = True

    if foundF90:
        env['f90_interface'] = 'y'
    elif env['f90_interface'] == 'y':
        print "ERROR: Couldn't find a suitable Fortran compiler to build the Fortran 90 interface"
        sys.exit(1)
    else:
        print "INFO: Skipping compilation of the Fortran 90 interface."

if env['F90'] == 'gfortran':
    env['FORTRANMODDIRPREFIX'] = '-J'
elif env['F90'] == 'g95':
    env['FORTRANMODDIRPREFIX'] = '-fmod='
elif env['F90'] == 'ifort':
    env['FORTRANMODDIRPREFIX'] = '-module '

env['FORTRANMODDIR'] = '${TARGET.dir}'

if env['CC'] == 'gcc':
    env['WIN32'] = False
elif env['CC'] == 'cl':
    env['WIN32'] = True
else:
    assert False

if env['boost_inc_dir']:
    env.Append(CPPPATH=env['boost_inc_dir'])

conf = Configure(env)

env['HAS_SSTREAM'] = conf.CheckCXXHeader('sstream', '<>')
env['HAS_TIMES_H'] = conf.CheckCHeader('sys/times.h', '""')
env['HAS_UNISTD_H'] = conf.CheckCHeader('unistd.h', '""')
env['HAS_MATH_H_ERF'] = conf.CheckDeclaration('erf', '#include <math.h>', 'C++')
env['HAS_BOOST_MATH'] = conf.CheckCXXHeader('boost/math/special_functions/erf.hpp', '<>')
env = conf.Finish()

if env['cantera_python_home'] == '' and env['prefix'] != defaults.prefix:
    env['cantera_python_home'] = env['prefix']

if env['python_package'] in ('full','default'):
    # Test to see if we can import the specified array module
    warnNoPython = False
    if env['python_array_home']:
        sys.path.append(env['python_array_home'])
    try:
        __import__(env['python_array'])
        print """INFO: Building the full Python package using %s.""" % env['python_array']
        env['python_package'] = 'full'
    except ImportError:
        if env['python_package'] == 'full':
            print ("""ERROR: Unable to find the array package """
                   """'%s' required by the full Python package.""" % env['python_array'])
            sys.exit(1)
        else:
            print ("""WARNING: Not building the full Python package """
                   """ because the array package '%s' could not be found.""" % env['python_array'])
            warnNoPython = True
            env['python_package'] = 'minimal'


if env['matlab_toolbox'] == 'y' and which(env['matlab_cmd']) is None:
    print """ERROR: Unable to find the Matlab executable '%s'""" % env['matlab_cmd']
    sys.exit(1)
elif env['matlab_toolbox'] == 'default':
    cmd = which(env['matlab_cmd'])
    if cmd is not None:
        env['matlab_toolbox'] = 'y'
        print """INFO: Building the Matlab toolbox using '%s'""" % cmd
    else:
        print """INFO: Skipping compilation of the Matlab toolbox. """


# **************************************
# *** Set options needed in config.h ***
# **************************************

configh = {'CANTERA_VERSION': quoted(env['cantera_version']),
           }

# Conditional defines
def cdefine(definevar, configvar, comp=True, value=1):
    if env.get(configvar) == comp:
        configh[definevar] = value
    else:
        configh[definevar] = None

cdefine('DEBUG_MODE', 'debug')
cdefine('WIN32', 'WIN32')

# Need to test all of these to see what platform.system() returns
configh['SOLARIS'] = 1 if env['OS'] == 'Solaris' else None
configh['DARWIN'] = 1 if env['OS'] == 'Darwin' else None
configh['CYGWIN'] = 1 if env['OS'] == 'Cygwin' else None
configh['WINMSVC'] = 1 if env['OS'] == 'Windows' else None
cdefine('NEEDS_GENERIC_TEMPL_STATIC_DECL', 'OS', 'Solaris')

cdefine('HAS_NUMPY', 'python_array', 'numpy')
cdefine('HAS_NUMARRAY', 'python_array', 'numarray')
cdefine('HAS_NUMERIC', 'python_array', 'numeric')
cdefine('HAS_NO_PYTHON', 'python_package', 'none')
configh['PYTHON_EXE'] = quoted(env['python_cmd']) if env['python_package'] != 'none' else None

cdefine('HAS_SUNDIALS', 'use_sundials', 'y')
if env['use_sundials']:
    cdefine('SUNDIALS_VERSION_22', 'sundials_version', '2.2')
    cdefine('SUNDIALS_VERSION_23', 'sundials_version', '2.3')
    cdefine('SUNDIALS_VERSION_24', 'sundials_version', '2.4')

cdefine('WITH_ELECTROLYTES', 'with_electrolytes')
cdefine('WITH_IDEAL_SOLUTIONS', 'with_ideal_solutions')
cdefine('WITH_LATTICE_SOLID', 'with_lattice_solid')
cdefine('WITH_METAL', 'with_metal')
cdefine('WITH_STOICH_SUBSTANCE', 'with_stoich_substance')
cdefine('WITH_SEMICONDUCTOR', 'with_semiconductor')
cdefine('WITH_PRIME', 'with_prime')
cdefine('H298MODIFY_CAPABILITY', 'with_n298modify_capability')
cdefine('WITH_PURE_FLUIDS', 'with_pure_fluids')
cdefine('WITH_HTML_LOGS', 'with_html_log_files')
cdefine('WITH_VCSNONIDEAL', 'with_vcsnonideal')

cdefine('LAPACK_FTN_STRING_LEN_AT_END', 'lapack_ftn_string_len_at_end')
cdefine('LAPACK_FTN_TRAILING_UNDERSCORE', 'lapack_ftn_trailing_underscore')
cdefine('FTN_TRAILING_UNDERSCORE', 'lapack_ftn_trailing_underscore')
cdefine('LAPACK_NAMES_LOWERCASE', 'lapack_names', 'lower')

configh['RXNPATH_FONT'] = quoted(env['rpfont'])
cdefine('THREAD_SAFE_CANTERA', 'build_thread_safe')
cdefine('HAS_SSTREAM', 'HAS_SSTREAM')
configh['CANTERA_DATA'] = quoted(os.path.join(env['prefix'], 'data'))

if not env['HAS_MATH_H_ERF']:
    if env['HAS_BOOST_MATH']:
        configh['USE_BOOST_MATH'] = 1
    else:
        print "Error: Couldn't find 'erf' in either <math.h> or Boost.Math"
        sys.exit(1)
else:
    configh['USE_BOOST_MATH'] = None

config_h = env.Command('config.h', 'config.h.in.scons', ConfigBuilder(configh))
env.AlwaysBuild(config_h)

# **********************************************
# *** Set additional configuration variables ***
# **********************************************
if env['blas_lapack_libs'] == '':
    # External BLAS/LAPACK were not given, so we need to compile them
    env['BUILD_BLAS_LAPACK'] = True
    env['blas_lapack_libs'] = ['ctlapack', 'ctblas']
else:
    ens['blas_lapack_libs'] = ','.split(env['blas_lapack_libs'])

if env['use_sundials'] == 'y' and env['sundials_include']:
    env.Append(CPPPATH=env['sundials_include'])
if env['use_sundials'] == 'y' and env['sundials_libdir']:
    env.Append(LIBPATH=env['sundials_libdir'])

env['ct_libdir'] = pjoin(env['prefix'], 'lib')
env['ct_bindir'] = pjoin(env['prefix'], 'bin')
env['ct_incdir'] = pjoin(env['prefix'], 'include', 'cantera')
env['ct_incroot'] = pjoin(env['prefix'], 'include')
env['ct_datadir'] = pjoin(env['prefix'], 'data')
env['ct_demodir'] = pjoin(env['prefix'], 'demos')
env['ct_templdir'] = pjoin(env['prefix'], 'templates')
env['ct_tutdir'] = pjoin(env['prefix'], 'tutorials')
env['ct_mandir'] = pjoin(env['prefix'], 'man1')
env['ct_matlab_dir'] = pjoin(env['prefix'], 'matlab', 'toolbox')

# *********************
# *** Build Cantera ***
# *********************

buildDir = 'build'
buildTargets = []
installTargets = []
demoTargets = []

env.SConsignFile()

env.Append(CPPPATH=[Dir(os.getcwd()),
                    Dir('build/include/cantera/kernel'),
                    Dir('build/include/cantera'),
                    Dir('build/include')],
           LIBPATH=[Dir('build/lib')],
           CCFLAGS=defaults.fPIC,
           FORTRANFLAGS=defaults.fPIC,
           F90FLAGS=defaults.fPIC)

# Put headers in place
for header in mglob(env, 'Cantera/cxx/include', 'h'):
    header = env.Command('build/include/cantera/%s' % header.name, header,
                         Copy('$TARGET', '$SOURCE'))
    buildTargets.extend(header)
    inst = env.Install('$ct_incdir', header)
    installTargets.extend(inst)

for header in mglob(env, 'Cantera/clib/src', 'h'):
    hcopy = env.Command('build/include/cantera/clib/%s' % header.name, header,
                        Copy('$TARGET', '$SOURCE'))
    buildTargets.append(header)
    inst = env.Install(pjoin('$ct_incdir','clib'), header)
    installTargets.extend(inst)

### List of libraries needed to link to Cantera ###
linkLibs = ['oneD','zeroD','equil','kinetics','transport',
            'thermo','ctnumerics','ctmath','tpx',
            'ctspectra','converters','ctbase']

if env['use_sundials'] == 'y':
    linkLibs.extend(('sundials_cvodes','sundials_nvecserial'))
else:
    linkLibs.append('cvode')

linkLibs.extend(env['blas_lapack_libs'])

if env['build_with_f2c']:
    linkLibs.append('ctf2c')
else:
    linkLibs.append('gfortran')

env['cantera_libs'] = linkLibs

configh = env.Command('build/include/cantera/config.h', 'config.h', Copy('$TARGET', '$SOURCE'))
inst = env.Install('$ct_incdir', configh)
installTargets.extend(inst)

# Add targets from the SConscript files in the various subdirectories
Export('env', 'buildDir', 'buildTargets', 'installTargets', 'demoTargets')

VariantDir('build/ext', 'ext', duplicate=0)
SConscript('build/ext/SConscript')

VariantDir('build/kernel', 'Cantera/src', duplicate=0)
SConscript('build/kernel/SConscript')

VariantDir('build/interfaces/clib', 'Cantera/clib', duplicate=0)
SConscript('build/interfaces/clib/SConscript')

VariantDir('build/interfaces/cxx', 'Cantera/cxx', duplicate=0)
SConscript('build/interfaces/cxx/SConscript')

if env['f90_interface'] == 'y':
    VariantDir('build/interfaces/fortran/', 'Cantera/fortran', duplicate=1)
    SConscript('build/interfaces/fortran/SConscript')

if env['python_package'] in ('full','minimal'):
    SConscript('Cantera/python/SConscript')

if env['matlab_toolbox'] == 'y':
    SConscript('Cantera/matlab/SConscript')

VariantDir('build/tools', 'tools', duplicate=0)
SConscript('build/tools/SConscript')

# Data files
inst = env.Install('$ct_datadir', mglob(env, pjoin('data','inputs'), 'cti', 'xml'))
installTargets.extend(inst)

# Install exp3to2.sh (used by some of the tests)
inst = env.Install('$ct_bindir', pjoin('bin', 'exp3to2.sh'))
installTargets.extend(inst)

### Meta-targets ###
build_demos = Alias('demos', demoTargets)

def postBuildMessage(target, source, env):
    print "**************************************************************"
    print "Compiliation complete. Type '[sudo] scons install' to install."
    print "**************************************************************"

finish_build = env.Command('finish_build', [], postBuildMessage)
env.Depends(finish_build, buildTargets)
build_cantera = Alias('build', finish_build)

Default('build')

def postInstallMessage(target, source, env):
    v = sys.version_info
    env['python_module_loc'] = pjoin(
        env['prefix'], 'lib', 'python%i.%i' % v[:2], 'site-packages')

    print """
Cantera has been successfully installed.

File locations:

    applications      %(ct_bindir)s
    library files     %(ct_libdir)s
    C++ headers       %(ct_incdir)s
    demos             %(ct_demodir)s
    data files        %(ct_datadir)s""" % env

    if env['python_package'] == 'full':
        print """
    Python package    %(python_module_loc)s""" % env
    elif warnNoPython:
        print """
    #################################################################
     WARNING: the Cantera Python package was not installed because a
     suitable array package (e.g. numpy) could not be found.
    #################################################################"""

    if env['matlab_toolbox'] == 'y':
        print """
    Matlab toolbox    %(ct_matlab_dir)s
    Matlab demos      %(ct_demodir)s/matlab
    Matlab tutorials  %(ct_tutdir)s/matlab

    An m-file to set the correct matlab path for Cantera is at:

        %(prefix)s/matlab/ctpath.m""" % env

    print """
    setup script      %(ct_bindir)s/setup_cantera

    The setup script configures the environment for Cantera. It is
    recommended that you run this script by typing:

        source %(ct_bindir)s/setup_cantera

    before using Cantera, or else include its contents in your shell
    login script.
    """ % env

finish_install = env.Command('finish_install', [], postInstallMessage)
env.Depends(finish_install, installTargets)
install_cantera = Alias('install', finish_install)

### Tests ###
if 'test' in COMMAND_LINE_TARGETS or 'test-clean' in COMMAND_LINE_TARGETS:
    SConscript('test_problems/SConscript')
