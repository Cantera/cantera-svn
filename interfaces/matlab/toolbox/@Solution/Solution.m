function s = Solution(src, id)
% Class :mat:func:`Solution` constructor.
%
% Class Solution represents solutions of multiple species. A
% solution is defined as a mixture of two or more constituents
% (species) that are completely mixed on molecular length
% scales. The macroscopic intensive thermodynamic state of a
% solution is specified by two thermodynamic properties (for
% example, the temperature and pressure), and the relative amounts
% of each species, which may be given as mole fractions or mass
% fractions. ::
%
%     >> s = Solution('input.xml'[, phase_name])
%
% constructs a Solution object from a specification contained in
% file ``input.xml``. Optionally, the name of the phase to be imported
% can be specified with ``phase_name``. If a
% Transport model is included in ``input.xml``, it will be included
% in the Solution instance with the default transport modeling as set
% in the input file. The transport modeling cannot be changed once the
% phase is loaded. To have different transport modeling, change the
% ``transport`` node in the CTML file, or ``transport`` property in
% the CTI file before loading the phase.
%
% Class :mat:func:`Solution` derives from three more basic classes, and most of
% its methods are inherited from these classes. These are:
%
%     * class :mat:func:`ThermoPhase`  -  composition information and thermodynamic properties
%     * class :mat:func:`Kinetics`     -  homogeneous kinetics
%     * class :mat:func:`Transport`    -  transport properties
%
% See also: :mat:func:`ThermoPhase`, :mat:func:`Kinetics`, :mat:func:`Transport`
%
% :param src:
%     Input string of CTI or CTML file name.
% :param id:
%     Optional, name of the phase to import.
% :return:
%     Instance of class :mat:func:`Solution`
%

doc = XML_Node('doc',src);
if nargin == 1
    node = findByName(doc,'phase');
else
    node = findByID(doc,id);
end
t = ThermoPhase(node);
k = Kinetics(node,t);
s.kin = k;
s.th = t;
tr = Transport(node,t,'default',4);
s.tr = tr;
s = class(s,'Solution',t,k,tr);
