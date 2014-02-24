function setPressure(self, P)
% Set the mixture pressure (Pa)
%
% :param self:
%     Instance of class :mat:func:`Mixture`
% :param P:
%     Pressure (Pa)
%

mixturemethods(6, mix_hndl(self), P);
