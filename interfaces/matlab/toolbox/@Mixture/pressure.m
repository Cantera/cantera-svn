function p = pressure(self)
% :param self:
%     Instance of class :mat:func:`Mixture`
% :return:
%     Pressure (Pa)
%

p = mixturemethods(26, mix_hndl(self));
