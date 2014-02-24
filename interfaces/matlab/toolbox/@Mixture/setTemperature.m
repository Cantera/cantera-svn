function setTemperature(self, T)
% Set the mixture Temperature (K)
%
% :param self:
%     Instance of class :mat:func:`Mixture`
% :param T:
%     Temperature (K)
%

mixturemethods(5, mix_hndl(self), T);
