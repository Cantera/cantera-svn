function setPhaseMoles(self, n, moles)
% Set the number of moles of phase number ``n`` to
% ``moles`` (kmol).
%
% :param self:
%     Instance of class :mat:func:`Mixture`
% :param n:
%     Phase number in the input
% :param moles:
%     Number of moles to add. Units: kmol
%

mixturemethods(7, mix_hndl(self), n, moles);
