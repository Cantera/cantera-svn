function x = findByName(root, name)
% Find an XML element by name
%
% :param root:
%     Instance of class :mat:func:`XML_Node`
% :param name:
%     String name of the element to search for.
% :return:
%     Instance of class :mat:func:`XML_Node`
%

index = ctmethods(10, 9, root.id, name);
x = XML_Node('','', index);
