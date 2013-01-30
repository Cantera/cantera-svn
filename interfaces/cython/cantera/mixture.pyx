cdef class Mixture:
    """

    Class Mixture represents mixtures of one or more phases of matter.  To
    construct a mixture, supply a list of phases to the constructor, each
    paired with the number of moles for that phase::

        >>> gas = cantera.Solution('gas.cti')
        >>> gas.speciesNames
        ['H2', 'H', 'O2', 'O', 'OH']
        >>> graphite = cantera.Solution('graphite.cti')
        >>> graphite.speciesNames
        ['C(g)']
        >>> mix = Mixture([(gas, 1.0), (graphite, 0.1)])
        >>> mix.speciesNames
        ['H2', 'H', 'O2', 'O', 'OH', 'C(g)']

    Note that the objects representing each phase compute only the intensive
    state of the phase -- they do not store any information on the amount of
    this phase. Mixture objects, on the other hand, represent the full
    extensive state.

    Mixture objects are 'lightweight' in the sense that they do not store
    parameters needed to compute thermodynamic or kinetic properties of the
    phases. These are contained in the ('heavyweight') phase objects. Multiple
    mixture objects may be constructed using the same set of phase objects.
    Each one stores its own state information locally, and synchronizes the
    phases objects whenever it requires phase properties.
    """

    cdef CxxMultiPhase* mix
    cdef list _phases

    def __cinit__(self, phases):
        self.mix = new CxxMultiPhase()
        self._phases = []

        cdef _SolutionBase phase
        for phase,moles in phases:
            self.mix.addPhase(phase.thermo, moles)
            self._phases.append(phase)

        self.mix.init()
        if self._phases:
            self.P = self._phases[0].P
            self.T = self._phases[0].T

    def __dealloc__(self):
        del self.mix

    def report(self):
        """
        Generate a report describing the thermodynamic state of this mixture. To
        print the report to the screen, simply call the mixture object. The
        following two statements are equivalent::

        >>> mix()
        >>> print(mix.report())
        """
        s = []
        for i,phase in enumerate(self._phases):
            s.append('************ Phase {0} ************'.format(phase.name))
            s.append('Moles:  {0}'.format(self.phaseMoles(i)))
            s.append(phase.report())

        return '\n'.join(s)

    def __call__(self):
        print(self.report())

    property nElements:
        """Total number of elements present in the mixture."""
        def __get__(self):
            return self.mix.nElements()

    cpdef int elementIndex(self, element) except *:
        """Index of element with name 'element'::

            >>> mix.elementIndex('H')
            2
        """
        if isinstance(element, (str, unicode)):
            index = self.mix.elementIndex(stringify(element))
        elif isinstance(element, (int, float)):
            index = <int>element
        else:
            raise TypeError("'element' must be a string or a number")

        if not 0 <= index < self.nElements:
            raise ValueError('No such element.')

        return index

    property nSpecies:
        """Number of species."""
        def __get__(self):
            return self.mix.nSpecies()

    def speciesName(self, k):
        """Name of the species with index *k*. Note that index numbers
        are assigned in order as phases are added."""
        return pystr(self.mix.speciesName(k))

    property speciesNames:
        def __get__(self):
            return [self.speciesName(k) for k in range(self.nSpecies)]

    def speciesIndex(self, phase, species):
        """
        :param phase:
            Phase object, index or name
        :param species:
            Species name or index

        Returns the global index of species *species* in phase *phase*.
        """
        p = self.phaseIndex(phase)

        if isinstance(species, (str, unicode)):
            k = self.phase(p).speciesIndex(species)
        elif isinstance(species, (int, float)):
            k = <int?>species
            if not 0 <= k < self.nSpecies:
                raise ValueError('Species index out of range')
        else:
            raise TypeError("'species' must be a string or number")

        return self.mix.speciesIndex(k, p)

    def nAtoms(self, k, m):
        """
        Number of atoms of element *m* in the species with global index *k*.
        The element may be referenced either by name or by index.

        >>> n = mix.nAtoms(3, 'H')
        4.0
        """
        if not 0 <= k < self.nSpecies:
            raise IndexError('Species index ({}) out of range (0 < {})'.format(k, self.nSpecies))
        return self.mix.nAtoms(k, self.elementIndex(m))

    property nPhases:
        """Number of phases"""
        def __get__(self):
            return len(self._phases)

    def phase(self, n):
        return self._phases[n]

    def phaseIndex(self, p):
        """Index of the phase named *p*."""
        if isinstance(p, ThermoPhase):
            p = p.name

        if isinstance(p, (int, float)):
            if p == int(p) and 0 <= p < self.nPhases:
                return int(p)
            else:
                raise IndexError("Phase index '{0}' out of range.".format(p))
        elif isinstance(p, (str, unicode)):
            for i, phase in enumerate(self._phases):
                if phase.name == p:
                    return i
        raise KeyError("No such phase: '{0}'".format(p))

    property phaseNames:
        """Names of all phases in the order added."""
        def __get__(self):
            return [phase.name for phase in self._phases]

    property T:
        """
        The Temperature [K] of all phases in the mixture. When set, the
        pressure of the mixture is held fixed.
        """
        def __get__(self):
            return self.mix.temperature()
        def __set__(self, T):
            self.mix.setTemperature(T)

    property minTemp:
        """
        The minimum temperature for which all species in multi-species
        solutions have valid thermo data. Stoichiometric phases are not
        considered in determining minTemp.
        """
        def __get__(self):
            return self.mix.minTemp()

    property maxTemp:
        """
        The maximum temperature for which all species in multi-species
        solutions have valid thermo data. Stoichiometric phases are not
        considered in determining maxTemp.
        """
        def __get__(self):
            return self.mix.maxTemp()

    property P:
        """The Pressure [Pa] of all phases in the mixture. When set, the
         temperature of the mixture is held fixed."""
        def __get__(self):
            return self.mix.pressure()
        def __set__(self, P):
            self.mix.setPressure(P)

    property charge:
        """The total charge in Coulombs, summed over all phases."""
        def __get__(self):
            return self.mix.charge()

    def phaseCharge(self, p):
        """The charge of phase *p* in Coulumbs."""
        return self.mix.phaseCharge(self.phaseIndex(p))

    def phaseMoles(self, p=None):
        """
        Moles in phase *p*, if *p* is specified, otherwise the number of
        moles in all phases.
        """
        if p is None:
            return [self.mix.phaseMoles(n) for n in range(self.nPhases)]
        else:
            return self.mix.phaseMoles(self.phaseIndex(p))

    def setPhaseMoles(self, p, moles):
        """
        Set the number of moles of phase *p* to *moles*
        """
        self.mix.setPhaseMoles(self.phaseIndex(p), moles)

    def speciesMoles(self, species=None):
        """
        Returns the number of moles of species *k* if *k* is specified,
        or the number of of moles of each species otherwise.
        """
        if species is not None:
            return self.mix.speciesMoles(species)

        cdef np.ndarray[np.double_t, ndim=1] data = np.empty(self.nSpecies)
        for k in range(self.nSpecies):
            data[k] = self.mix.speciesMoles(k)
        return data

    def setSpeciesMoles(self, moles):
        """
        Set the moles of the species [kmol]. The moles may be specified either
        as a string, or as an array. If an array is used, it must be
        dimensioned at least as large as the total number of species in the
        mixture. Note that the species may belong to any phase, and
        unspecified species are set to zero.

        >>> mix.setSpeciesMoles('C(s):1.0, CH4:2.0, O2:0.2')

        """
        if isinstance(moles, (str, unicode)):
            self.mix.setMolesByName(stringify(moles))
            return

        if len(moles) != self.nSpecies:
            raise ValueError('mole array must be of length nSpecies')

        cdef np.ndarray[np.double_t, ndim=1] data = \
            np.ascontiguousarray(moles, dtype=np.double)
        self.mix.setMoles(&data[0])

    def elementMoles(self, e):
        """
        Total number of moles of element *e*, summed over all species.
        The element may be referenced either by index number or by name.
        """
        return self.mix.elementMoles(self.elementIndex(e))

    property chem_potentials:
        """The chemical potentials of all species [J/kmol]."""
        def __get__(self):
            cdef np.ndarray[np.double_t, ndim=1] data = np.empty(self.nSpecies)
            self.mix.getChemPotentials(&data[0])
            return data

    def equilibrate(self, XY, solver='vcs', rtol=1e-9, maxsteps=1000,
                    maxiter=100, estimateEquil=0, printlevel=0, loglevel=0):
        """
        Set to a state of chemical equilibrium holding property pair *XY*
        constant. This method uses a version of the VCS algorithm to find the
        composition that minimizes the total Gibbs free energy of the mixture,
        subject to element conservation constraints. For a description of the
        theory, see Smith and Missen, "Chemical Reaction Equilibrium."

        :param XY:
            A two-letter string, which must be one of the set::

                ['TP', 'HP', 'SP']
        :param solver:
            Set to either 'vcs' or 'gibbs' to choose implementation
            of the solver to use. 'vcs' uses the solver implemented in the
            C++ class 'VCSnonideal', and 'gibbs' uses the one implemented
            in class 'MultiPhaseEquil'.
        :param rtol:
            Error tolerance. Iteration will continue until (Delta mu)/RT is
            less than this value for each reaction. Note that this default is
            very conservative, and good equilibrium solutions may be obtained
            with larger error tolerances.
        :param maxsteps:
            Maximum number of steps to take while solving the equilibrium
            problem for specified *T* and *P*.
        :param maxiter:
            Maximum number of temperature and/or pressure iterations.
            This is only relevant if a property pair other than (T,P) is
            specified.
        :param estimateEquil:
            Flag indicating whether the solver should estimate its own initial
            condition. If 0, the initial mole fraction vector in the phase
            objects are used as the initial condition. If 1, the initial mole
            fraction vector is used if the element abundances are satisfied.
            if -1, the initial mole fraction vector is thrown out, and an
            estimate is formulated.
        :param printlevel:
            Determines the amount of output displayed during the solution
            process. 0 indicates no output, while larger numbers produce
            successively more verbose information.
        :param loglevel:
            Controls the amount of diagnostic output written to an HTML log
            file. If loglevel = 0, no diagnostic output is written. For
            values > 0, more detailed information is written to the log file as
            loglevel increases. The default log file name is
            "equilibrium_log.html", but if this file exists, the log
            information will be written to "equilibrium_log{n}.html",
            where {n} is an integer chosen to avoid overwriting existing
            log files.
        """
        if solver == 'vcs':
            iSolver = 2
        elif solver == 'gibbs':
            iSolver = 1
        else:
            raise ValueError('Unrecognized equilibrium solver '
                             'specified: "{}"'.format(solver))

        vcs_equilibrate(deref(self.mix), stringify(XY).c_str(), estimateEquil,
                        printlevel, iSolver, rtol, maxsteps, maxiter, loglevel)
