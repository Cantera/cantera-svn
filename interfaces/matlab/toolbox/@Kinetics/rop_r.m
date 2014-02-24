function q = rop_r(a)
% Reverse rates of progress for all reactions.
%
%    See also: :mat:func:`rop_f`, :mat:func:`rop_net`, :mat:func:`rop`
%
% :param a:
%     Instance of class :mat:func:`Kinetics` (or another
%     object containing an instance of that class)
%     for which forward rate constants are desired.
% :return:
%     Returns a column vector of the reverse rates of progress
%     for all reactions. If this function
%     is called with no output argument, a bar graph is produced.
%

q = kinetics_get(a.id,12,0);
if nargout == 0
    figure
    set(gcf,'Name','Reverse Rates of Progress')
    bar(q)
    xlabel('Reaction Number')
    ylabel('Reverse Rate of Progress [kmol/m^3]')
    title('Reverse Rates of Progress')
end
