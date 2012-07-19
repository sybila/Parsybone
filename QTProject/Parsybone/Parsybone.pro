TEMPLATE = app
CONFIG += console
CONFIG -= qt

SOURCES += \
    ../../main.cpp

INCLUDEPATH += "C:\Program Files\boost_1_46_1"

HEADERS += \
    ../../auxiliary/user_options.hpp \
    ../../auxiliary/time_manager.hpp \
    ../../auxiliary/template.hpp \
    ../../auxiliary/output_streamer.hpp \
    ../../auxiliary/data_types.hpp \
    ../../auxiliary/common_functions.hpp \
    ../../reforging/product_structure.hpp \
    ../../reforging/product_builder.hpp \
    ../../reforging/parametrized_structure_builder.hpp \
    ../../reforging/parametrized_structure.hpp \
    ../../reforging/parametrizations_builder.hpp \
    ../../reforging/parametrizations_holder.hpp \
    ../../reforging/graph_interface.hpp \
    ../../reforging/color_storage.hpp \
    ../../reforging/basic_structure_builder.hpp \
    ../../reforging/basic_structure.hpp \
    ../../reforging/automaton_structure.hpp \
    ../../reforging/automaton_interface.hpp \
    ../../reforging/automaton_builder.hpp \
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
    ../../reforging/construction_manager.hpp \
    ../../reforging/constrains_parser.hpp \
    ../../reforging/parametrizations_holder.hpp \
    ../../reforging/parametrizations_builder.hpp

