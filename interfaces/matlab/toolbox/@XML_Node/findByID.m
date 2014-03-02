function x = findByID(root, id)
% Find an XML element by ID
%
% :param root:
%     Instance of class :mat:func:`XML_Node`
% :param id:
%     String ID of the element to search for.
% :return:
%     Instance of class :mat:func:`XML_Node`
%

index = ctmethods(10, 8, root.id, id);
x = XML_Node('','', index);
