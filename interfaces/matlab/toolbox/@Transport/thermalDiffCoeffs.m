function v = thermalDiffCoeffs(a)
% Thermal diffusion coefficients.
%
% Returns a column vector with the thermal diffusion
% coefficients. Object ``gas`` must belong to a class derived from
% Transport, and that was constructed by specifying the ``'Multi'``
% option. If ``'Multi'`` was not specified, the returned values will
% all be zero.
%
% :param a:
%     Instance of class :mat:func:`Transport` (or another
%     object containing an instance of that class)
%     for which the thermal diffusion coefficients are desired.
% :return:
%     Vector of thermal diffusion coefficients of length nSpecies
%

v = trans_get(a.id, 12, nSpecies(a.th));
