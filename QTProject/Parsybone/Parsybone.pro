TEMPLATE = app
CONFIG += console
CONFIG -= qt

INCLUDEPATH += "C:/Program Files/boost_1_50_0"

QMAKE_CXXFLAGS += -std=gnu++0x

HEADERS += \
    ../../auxiliary/user_options.hpp \
    ../../auxiliary/time_manager.hpp \
    ../../auxiliary/template.hpp \
    ../../auxiliary/output_streamer.hpp \
    ../../auxiliary/data_types.hpp \
    ../../auxiliary/common_functions.hpp \
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
    ../../construction/product_structure.hpp \
    ../../construction/product_builder.hpp \
    ../../construction/parametrized_structure_builder.hpp \
    ../../construction/parametrized_structure.hpp \
    ../../construction/graph_interface.hpp \
    ../../construction/construction_manager.hpp \
    ../../construction/basic_structure_builder.hpp \
    ../../construction/basic_structure.hpp \
    ../../construction/automaton_structure.hpp \
    ../../construction/automaton_interface.hpp \
    ../../construction/automaton_builder.hpp \
    ../../construction/construction_holder.hpp \
    ../../construction/state_property.hpp \
    ../../construction/labeling_holder.hpp \
    ../../construction/labeling_builder.hpp \
    ../../construction/parametrizations_builder.hpp \
    ../../construction/parametrizations_holder.hpp \
    ../../synthesis/color_storage.hpp \
    ../../synthesis/paramset_helper.hpp \
    ../../synthesis/witness_searcher.hpp \
    ../../synthesis/synthesis_manager.hpp \
    ../../synthesis/split_manager.hpp \
    ../../synthesis/paramset_helper.hpp \
    ../../synthesis/output_manager.hpp \
    ../../synthesis/model_checker.hpp \
    ../../synthesis/coloring_analyzer.hpp \
    ../../synthesis/color_storage.hpp \
    ../../synthesis/robustness_compute.hpp \
    ../../parsing/xml_helper.hpp \
    ../../parsing/translator.hpp \
    ../../parsing/property_parser.hpp \
    ../../parsing/network_parser.hpp \
    ../../parsing/formulae_parser.hpp

