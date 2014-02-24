function v = multiDiffCoeffs(a)
% Multicomponent diffusion coefficients (m^2/s) of all of the
% species.
%
% Returns a column vector with the multicomponent diffusion
% coefficients. Object ``gas`` must belong to a class derived from
% Transport, and that was constructed by specifying the ``'Multi'``
% option. If ``'Multi'`` was not specified, you will get the
% error message ::
%
%     **** Method getMultiDiffCoeffs not implemented. ****
%
% In this case, try method :mat:func:`mixDiffCoeffs`, or create a
% new gas mixture model that uses a mixture-averaged transport manager,
% for example::
%
%     >> gas = GRI30('Multi');
%
% :param a:
%     Instance of class :mat:func:`Transport` (or another
%     object containing an instance of that class)
%     for which multicomponent diffusion coefficients are desired.
% :return:
%     Vector of length nSpecies with the multicomponent diffusion
%     coefficients.
%

v = trans_get(a.id, 22, nSpecies(a.th));

