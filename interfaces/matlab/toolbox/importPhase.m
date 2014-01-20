function s = importPhase(file, name)
% Import a phase from a CTI file. See class :mat:func:`Solution` and
% :ref:`sec-phases`.
%
% :param file:
%     CTI file containing phase definition
% :param name:
%     Name of the phase
% :return:
%     Instance of class :mat:func:`Solution`
%

if nargin == 1
    s = Solution(file);
elseif nargin == 2
    s = Solution(file, name);
end
