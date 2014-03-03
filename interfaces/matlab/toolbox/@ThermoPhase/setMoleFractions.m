function setMoleFractions(tp,x,norm)
%
%
%

if isa(x,'double')
    if nargin == 3
        if strcmp(norm,'nonorm')
            phase_set(tp.tp_id,22,x);
        else
            phase_set(tp.tp_id,20,x);
        end
    else
        phase_set(tp.tp_id,20,x);
    end
    %
    % string input
    %
elseif isa(x,'char')
    phase_set(tp.tp_id,30,x);
end
