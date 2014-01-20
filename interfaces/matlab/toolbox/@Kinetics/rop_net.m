function q = rop_net(a)
% ROP_NET  Net rates of progress for all reactions.
%
%    q = rop_net(a)
%
%        Returns a column vector of the net rates of progress
%        for all reactions.
%
%    See also: rop_f, rop_r.
%
q = kinetics_get(a.id,13,0);
if nargout == 0
    figure
    set(gcf,'Name','Rates of Progress')
    bar(q)
    xlabel('Reaction Number')
    ylabel('Net Rate of Progress [kmol/m^3]')
    title('Net Rates of Progress')
end
