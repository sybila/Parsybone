TEMPLATE = app
CONFIG += console
CONFIG -= qt

SOURCES += \
    ../../main.cpp

INCLUDEPATH += "C:/Program Files/boost_1_46_1"

HEADERS += \
    ../../auxiliary/user_options.hpp \
    ../../auxiliary/time_manager.hpp \
    ../../auxiliary/template.hpp \
    ../../auxiliary/output_streamer.hpp \
    ../../auxiliary/data_types.hpp \
    ../../auxiliary/common_functions.hpp \
    ../../construction/product_structure.hpp \
    ../../construction/product_builder.hpp \
    ../../construction/parametrized_structure_builder.hpp \
    ../../construction/parametrized_structure.hpp \
    ../../construction/parametrizations_builder.hpp \
    ../../construction/parametrizations_holder.hpp \
    ../../construction/graph_interface.hpp \
    ../../construction/color_storage.hpp \
    ../../construction/basic_structure_builder.hpp \
    ../../construction/basic_structure.hpp \
    ../../construction/automaton_structure.hpp \
    ../../construction/automaton_interface.hpp \
    ../../construction/automaton_builder.hpp \
    ../../results/witness_searcher.hpp \
    ../../results/per_color_storage.hpp \
    ../../results/output_manager.hpp \
    ../../results/coloring_analyzer.hpp \
    ../../coloring/synthesis_manager.hpp \
    ../../coloring/split_manager.hpp \
    ../../coloring/parameters_functions.hpp \
    ../../coloring/model_checker.hpp \
    ../../parsing/model_parser.hpp \
    ../../parsing/model.hpp \
    ../../parsing/coloring_parser.hpp \
    ../../parsing/argument_parser.hpp \
    ../../parsing/rapidxml-1.13/rapidxml_utils.hpp \
    ../../parsing/rapidxml-1.13/rapidxml_print.hpp \
    ../../parsing/rapidxml-1.13/rapidxml_iterators.hpp \
    ../../parsing/rapidxml-1.13/rapidxml.hpp \
    ../../parsing/parsing_manager.hpp \
    ../../construction/construction_manager.hpp \
    ../../construction/constrains_parser.hpp \
    ../../construction/parametrizations_holder.hpp \
    ../../construction/parametrizations_builder.hpp \
    ../../construction/product_structure.hpp \
    ../../construction/product_builder.hpp \
    ../../construction/parametrized_structure_builder.hpp \
    ../../construction/parametrized_structure.hpp \
    ../../construction/parametrizations_holder.hpp \
    ../../construction/parametrizations_builder.hpp \
    ../../construction/graph_interface.hpp \
    ../../construction/construction_manager.hpp \
    ../../construction/constrains_parser.hpp \
    ../../construction/color_storage.hpp \
    ../../construction/basic_structure_builder.hpp \
    ../../construction/basic_structure.hpp \
    ../../construction/automaton_structure.hpp \
    ../../construction/automaton_interface.hpp \
    ../../construction/automaton_builder.hpp \
    ../../construction/construction_holder.hpp \
    ../../construction/state_property.hpp \
    ../../construction/transition_property.hpp

