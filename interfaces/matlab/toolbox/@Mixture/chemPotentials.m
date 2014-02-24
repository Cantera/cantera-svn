function mu = chemPotentials(self)
% Chemical potentials of all species in all phases in an instance of
% class :mat:func:`Mixture`.
%
% :param self:
%     Instance of class :mat:func:`Mixture`
%

mu = mixturemethods(41, mix_hndl(self));
