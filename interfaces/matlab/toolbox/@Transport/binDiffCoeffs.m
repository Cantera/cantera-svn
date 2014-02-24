function v = binDiffCoeffs(a)
% Binary diffusion coefficients (m^2/s).
%
% :param a:
%     Instance of class :mat:func:`Transport` (or another
%     object containing an instance of that class)
%     for which binary diffusion coefficients are desired.
% :return:
%     A matrix of binary diffusion coefficients.
%     The matrix is symmetric: d(i,j) = d(j,i).
%

v = trans_get(a.id, 21, nSpecies(a.th));
