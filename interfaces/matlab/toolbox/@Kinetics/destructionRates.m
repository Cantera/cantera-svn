function ddot = destructionRates(a)
% Chemical destruction rates (kmol/m^3/s).
%
% See also: :mat:func:`creationRates`, :mat:func:`netProdRates`
%
% :param a:
%     Instance of class :mat:func:`Kinetics` (or another
%     object containing an instance of that class)
%     for which destruction rates are desired.
% :return:
%     Returns a column vector of the destruction rates of all
%     species. If the output is not assigned to a variable, a
%     bar graph is produced.
%

ddot = kinetics_get(a.id,22,0);
if nargout == 0
    figure
    set(gcf,'Name','Destruction Rates')
    bar(ddot)
    xlabel('Species Number')
    ylabel('Destruction Rate (kmol/m^3/s)')
    title('Species Chemical Destruction Rates')
end
