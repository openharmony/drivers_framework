import json
import sys
import unittest
from os import remove

sys.path.insert(0, '..')

try:
    from _header_parser import HeaderParser
    from CppHeaderParser import *
finally:
    pass


class HeaderParserTestCase(unittest.TestCase):

    def test_extract_path_and_file(self):
        parser = HeaderParser()

        parser._extract_path_and_file('include/test/audio_manager.h')
        self.assertEqual(parser._header_dict['path'], 'include/test')
        self.assertEqual(parser._header_dict['name'], 'audio_manager.h')

        parser._extract_path_and_file('./include/test/audio_manager.h')
        self.assertEqual(parser._header_dict['path'], './include/test')
        self.assertEqual(parser._header_dict['name'], 'audio_manager.h')

        parser._extract_path_and_file('../include/test/audio_manager.h')
        self.assertEqual(parser._header_dict['path'], '../include/test')
        self.assertEqual(parser._header_dict['name'], 'audio_manager.h')

        parser._extract_path_and_file('\\include\\test\\audio_manager.h')
        self.assertEqual(parser._header_dict['path'], '\\include\\test')
        self.assertEqual(parser._header_dict['name'], 'audio_manager.h')

        parser._extract_path_and_file('d:\\include\\test\\audio_manager.h')
        self.assertEqual(parser._header_dict['path'], 'd:\\include\\test')
        self.assertEqual(parser._header_dict['name'], 'audio_manager.h')

    def test_extract_include(self):
        parser = HeaderParser()
        parser._extract_include(['"buffer_handle.h"', '"buffer_type.h"'])
        self.assertSequenceEqual(parser._header_dict["import"], ['buffer_handle.h', 'buffer_type.h'])

    def test_extract_include_system_file(self):
        parser = HeaderParser()
        parser._extract_include(['<fcntl.h>', '<sys/types.h>', '"buffer_handle.h"', '"buffer_type.h"'])
        self.assertSequenceEqual(parser._header_dict["import"], ['buffer_handle.h', 'buffer_type.h'])

    def test_extract_enum(self):
        header_file = """
            typedef enum {
                   DISPLAY_SUCCESS = 0,
                   DISPLAY_FAILURE = 1,
            } DispErrCode;
        """
        hjson = json.loads(CppHeaderParser.CppHeader(header_file, "string").toJSON())
        parser = HeaderParser()
        parser._extract_enum(hjson["enums"])
        self.assertSequenceEqual(parser._header_dict["enum"], [{'name': 'DispErrCode',
                                                                'members': [{'name': 'DISPLAY_SUCCESS', 'value': 0},
                                                                            {'name': 'DISPLAY_FAILURE', 'value': 1}]}])

    def test_extract_enum_with_type(self):
        header_file = """
            enum CamRetCode : int32_t {
                NO_ERROR = 0,
                CAMERA_BUSY = 1,
            };
        """
        hjson = json.loads(CppHeaderParser.CppHeader(header_file, "string").toJSON())
        parser = HeaderParser()
        parser._extract_enum(hjson["enums"])
        self.assertSequenceEqual(parser._header_dict["enum"], [{'name': 'CamRetCode',
                                                                'members': [{'name': 'NO_ERROR', 'value': 0},
                                                                            {'name': 'CAMERA_BUSY', 'value': 1}]}])

    def test_extract_enum_unexpected_value(self):
        header_file = """
            typedef enum {
                   DISPLAY_SUCCESS = 0x1234u,
                   DISPLAY_FAILURE = -1,
                   HBM_USE_CPU_READ = (1 << 0),
            } DispErrCode;
        """
        hjson = json.loads(CppHeaderParser.CppHeader(header_file, "string").toJSON())
        hjson["enums"][0]["filename"] = "test_header_parser.py"
        hjson["enums"][0]["line_number"] = 0
        parser = HeaderParser()
        parser._extract_enum(hjson["enums"])
        self.assertSequenceEqual(
            parser._header_dict["enum"], [{'name': 'DispErrCode',
                                           'members': [
                                               {'name': 'DISPLAY_SUCCESS', 'value': '0x1234 u // unexpected \'0x\''},
                                               {'name': 'DISPLAY_FAILURE', 'value': '- 1 // unexpected \'-\''},
                                               {'name': 'HBM_USE_CPU_READ', 'value': '( 1 << 0 ) // unexpected \'(\''}
                                           ]}])

    def test_extract_enum_without_name(self):
        header_file = """
            enum {
                   DISPLAY_SUCCESS = 1,
                   DISPLAY_FAILURE = 2,
            };
            enum {
                   DISPLAY_START = 1,
                   DISPLAY_END = 2,
            };
        """
        hjson = json.loads(CppHeaderParser.CppHeader(header_file, "string").toJSON())
        parser = HeaderParser()
        parser._extract_enum(hjson["enums"])
        self.assertSequenceEqual(parser._header_dict["enum"], [{'name': 'LostName_0',
                                                                'members': [
                                                                    {'name': 'DISPLAY_SUCCESS', 'value': 1},
                                                                    {'name': 'DISPLAY_FAILURE', 'value': 2}
                                                                ]},
                                                               {'name': 'LostName_1',
                                                                'members': [
                                                                    {'name': 'DISPLAY_START', 'value': 1},
                                                                    {'name': 'DISPLAY_END', 'value': 2}
                                                                ]}])

    def test_extract_union(self):
        header_file = """
            union SceneDesc {
                uint32_t id;
                const char *desc;
            };
        """
        parser = HeaderParser()
        hjson = json.loads(CppHeaderParser.CppHeader(header_file, "string").toJSON())
        parser._extract_union(hjson["classes"]["SceneDesc"])
        self.assertSequenceEqual(parser._header_dict["union"], [{'name': 'SceneDesc',
                                                                 'type': 'union',
                                                                 'members': [
                                                                     {'name': 'id', 'type': 'uint32_t'},
                                                                     {'name': 'desc', 'type': 'const char *'}
                                                                 ]}])

    def test_extract_union_without_name(self):
        header_file = """
            union {
                uint32_t id;
                const char *desc;
            };
            union {
                uint32_t id2;
                const char *desc2;
            };
        """
        parser = HeaderParser()
        hjson = json.loads(CppHeaderParser.CppHeader(header_file, "string").toJSON())
        parser._extract_union(hjson["classes"]["<anon-union-1>"])
        parser._extract_union(hjson["classes"]["<anon-union-2>"])
        self.assertSequenceEqual(parser._header_dict["union"], [{'name': '<anon-union-1>',
                                                                 'type': 'union',
                                                                 'members': [
                                                                     {'name': 'id', 'type': 'uint32_t'},
                                                                     {'name': 'desc', 'type': 'const char *'}
                                                                 ]},
                                                                {'name': '<anon-union-2>',
                                                                 'type': 'union',
                                                                 'members': [
                                                                     {'name': 'id2', 'type': 'uint32_t'},
                                                                     {'name': 'desc2', 'type': 'const char *'}
                                                                 ]}
                                                                ])

    def test_extract_struct_include_union(self):
        header_file = """
            struct AudioSceneDescriptor {
                union SceneDesc {
                    uint32_t id;
                    const char *desc;
                } scene;
            };
        """
        parser = HeaderParser()
        hjson = json.loads(CppHeaderParser.CppHeader(header_file, "string").toJSON())
        parser._extract_union(hjson["classes"]["AudioSceneDescriptor::SceneDesc"])
        self.assertSequenceEqual(parser._header_dict["union"], [{'name': 'SceneDesc',
                                                                 'type': 'union',
                                                                 'members': [
                                                                     {'name': 'id', 'type': 'uint32_t'},
                                                                     {'name': 'desc', 'type': 'const char *'}
                                                                 ]}])
        parser._extract_struct(hjson["classes"]["AudioSceneDescriptor"])
        self.assertSequenceEqual(parser._header_dict["struct"], [{'name': 'AudioSceneDescriptor',
                                                                  'type': 'struct',
                                                                  'members': [
                                                                      {'name': 'scene', 'type': 'SceneDesc'}
                                                                  ]}])

    def test_extract_struct(self):
        header_file = """
            typedef struct {
                bool succeed;
                int8_t end;
                float rate;
                double rate2;
                cstring desc;
                std::string location;
                uint8_t status; 
                uint16_t busType;
                int32_t num;
                uint32_t count;
                uint64_t timestamp;
                void path;
                char chipInfo[32];
                enum RetStatus status;
                struct EvtPack package;
                struct EvtPack *pkgPtr;
            } DevAbility;
        """
        parser = HeaderParser()
        hjson = json.loads(CppHeaderParser.CppHeader(header_file, "string").toJSON())
        parser._extract_struct(hjson["classes"]["DevAbility"])
        self.assertSequenceEqual(parser._header_dict["struct"], [{'name': 'DevAbility',
                                                                  'type': 'struct',
                                                                  'members': [
                                                                      {'name': 'succeed', 'type': 'bool'},
                                                                      {'name': 'end', 'type': 'int8_t'},
                                                                      {'name': 'rate', 'type': 'float'},
                                                                      {'name': 'rate2', 'type': 'double'},
                                                                      {'name': 'desc', 'type': 'cstring'},
                                                                      {'name': 'location', 'type': 'std::string'},
                                                                      {'name': 'status', 'type': 'uint8_t'},
                                                                      {'name': 'busType', 'type': 'uint16_t'},
                                                                      {'name': 'num', 'type': 'int32_t'},
                                                                      {'name': 'count', 'type': 'uint32_t'},
                                                                      {'name': 'timestamp', 'type': 'uint64_t'},
                                                                      {'name': 'path', 'type': 'void'},
                                                                      {'name': 'chipInfo', 'type': 'char *'},
                                                                      {'name': 'status', 'type': 'RetStatus'},
                                                                      {'name': 'package', 'type': 'struct EvtPack'},
                                                                      {'name': 'pkgPtr', 'type': 'struct EvtPack *'}
                                                                  ]}])

    def test_extract_struct_with_enum_pointer(self):
        header_file = """
            typedef struct {
                enum Status *staPtr;
                enum Test *testPtr;
            } DevAbility;
        """
        file_name = "header_file.h"
        with open(file_name, 'w') as f:
            f.write(header_file)

        parser = HeaderParser()
        back_file = parser._pre_handle(file_name)
        hjson = json.loads(CppHeaderParser.CppHeader(file_name).toJSON())
        remove(back_file)
        remove(file_name)
        parser._extract_struct(hjson["classes"]["DevAbility"])
        self.assertSequenceEqual(parser._header_dict["struct"], [{'name': 'DevAbility',
                                                                  'type': 'struct',
                                                                  'members': [
                                                                      {'name': 'staPtr', 'type': 'Status_ENUM_POINTER'},
                                                                      {'name': 'testPtr', 'type': 'Test_ENUM_POINTER'}
                                                                  ]}])

    def test_extract_struct_with_macro(self):
        header_file = """
            #define CODE_SIZE 32
            #define DIV_ROUND_UP(nr, d) (((nr) + (d) - 1) / (d))
            #define BYTE_HAS_BITS 8
            #define BITS_TO_UINT64(count)    DIV_ROUND_UP(count, BYTE_HAS_BITS * sizeof(unsigned long))

            typedef struct {
                unsigned long absCode[BITS_TO_UINT64(CODE_SIZE)];
            } DevAbility;
        """
        file_name = "header_file.h"
        with open(file_name, 'w') as f:
            f.write(header_file)

        parser = HeaderParser()
        back_file = parser._pre_handle(file_name)
        hjson = json.loads(CppHeaderParser.CppHeader(file_name).toJSON())
        remove(back_file)
        remove(file_name)

        parser._extract_struct(hjson["classes"]["DevAbility"])
        self.assertSequenceEqual(parser._header_dict["struct"], [{'name': 'DevAbility',
                                                                  'type': 'struct',
                                                                  'members': [
                                                                      {'name': 'absCode', 'type': 'unsigned long *'}
                                                                  ]}])

    def test_extract_struct_include_union_without_name(self):
        header_file = """
            struct AudioSceneDescriptor {
                union {
                    uint32_t id;
                    const char *desc;
                };
            };
        """
        parser = HeaderParser()
        hjson = json.loads(CppHeaderParser.CppHeader(header_file, "string").toJSON())
        parser._extract_union(hjson["classes"]["AudioSceneDescriptor::<anon-union-1>"])
        self.assertSequenceEqual(parser._header_dict["union"], [{'name': '<anon-union-1>',
                                                                 'type': 'union',
                                                                 'members': [
                                                                     {'name': 'id', 'type': 'uint32_t'},
                                                                     {'name': 'desc', 'type': 'const char *'}
                                                                 ]}])
        parser._extract_struct(hjson["classes"]["AudioSceneDescriptor"])
        self.assertSequenceEqual(parser._header_dict["struct"], [{'name': 'AudioSceneDescriptor',
                                                                  'type': 'struct',
                                                                  'members': [
                                                                      {'name': '', 'type': '<anon-union-1>'}
                                                                  ]}])

    def test_extract_interface_with_method(self):
        header_file = """
            typedef struct {
                int32_t SetPowerStatus(uint32_t devIndex);
                int32_t RunExtraCommand(uint32_t, InputExtraCmd *);
            } InputController;
        """
        parser = HeaderParser()
        hjson = json.loads(CppHeaderParser.CppHeader(header_file, "string").toJSON())
        parser._extract_interface(hjson["classes"]["InputController"])
        self.assertSequenceEqual(parser._header_dict["interface"], [{'name': 'InputController',
                                                                     'members': [
                                                                         {'name': 'SetPowerStatus',
                                                                          'params': [{'name': 'devIndex',
                                                                                      'type': 'uint32_t'}]},
                                                                         {'name': 'RunExtraCommand',
                                                                          'params': [{'name': 'rand_name_0',
                                                                                      'type': 'uint32_t'},
                                                                                     {'name': 'rand_name_1',
                                                                                      'type': 'InputExtraCmd *'}
                                                                                     ]}]}])

    def test_extract_interface_with_function_point(self):
        header_file = """
            typedef struct {
                int32_t (*SetPowerStatus)(uint32_t devIndex);
                int32_t (*RunExtraCommand)(uint32_t, InputExtraCmd *);
                int32_t (*RunCapacitanceTest)(uint32_t devIndex,
                        uint32_t testType);
            } InputController;
        """
        parser = HeaderParser()
        hjson = json.loads(CppHeaderParser.CppHeader(header_file, "string").toJSON())
        parser._extract_interface(hjson["classes"]["InputController"])
        self.assertSequenceEqual(parser._header_dict["interface"], [{'name': 'InputController',
                                                                     'members': [
                                                                         {'name': 'SetPowerStatus',
                                                                          'params': [{'name': 'devIndex',
                                                                                      'type': 'uint32_t'}]},
                                                                         {'name': 'RunExtraCommand',
                                                                          'params': [{'name': 'rand_name_0',
                                                                                      'type': 'uint32_t'},
                                                                                     {'name': 'rand_name_1',
                                                                                      'type': 'InputExtraCmd *'}
                                                                                     ]},
                                                                         {'name': 'RunCapacitanceTest',
                                                                          'params': [{'name': 'devIndex',
                                                                                      'type': 'uint32_t'},
                                                                                     {'name': 'testType',
                                                                                      'type': 'uint32_t'}
                                                                                     ]}
                                                                     ]}])

    def test_extract_interface_callback(self):
        # todo
        pass

    def test_extract_interface_with_function_pointer_and_properties(self):
        # todo
        pass

    def test_extract_typedef(self):
        header_file = """
            typedef void (*HotPlugCallback)(uint32_t devId, bool connected);
            typedef void *AudioHandle;
        """
        parser = HeaderParser()
        hjson = json.loads(CppHeaderParser.CppHeader(header_file, "string").toJSON())
        parser._extract_typedef(hjson["typedefs"])
        self.assertSequenceEqual(parser._header_dict["typedef"],
                                 [{'name': 'HotPlugCallback',
                                   'type': '/* unsupported function pointer type: HotPlugCallback */'},
                                  {'name': 'AudioHandle',
                                   'type': 'void *'}])

    def test_checkout_function_pointer_param(self):
        header_file = """
            typedef struct {
                int32_t (*Second)(bool vb, const int8_t vi8, const int16_t *vi16);
                int32_t (*Minute)(struct Test st, const struct Test st);
                int32_t (*Hour)(const struct Test *st, const struct Test * st);
                int32_t (*Day)(const struct Test  *  st, const struct Test* st);
                int32_t (*Month)(std::string str, unsigned int a, unsigned short, enum Test *c);
                int32_t (*Year)(unsigned int **out1, const struct Test **out2);
            } IFoo;
        """
        parser = HeaderParser()
        hjson = json.loads(CppHeaderParser.CppHeader(header_file, "string").toJSON())
        stack = hjson["classes"]["IFoo"]
        second = stack["properties"]["public"][0]
        params = parser._checkout_function_pointer_param(second["type"])
        self.assertSequenceEqual(params, [{'name': 'vb', 'type': 'bool'},
                                          {'name': 'vi8', 'type': 'int8_t'},
                                          {'name': 'vi16', 'type': 'int16_t *'}])

        minute = stack["properties"]["public"][1]
        params = parser._checkout_function_pointer_param(minute["type"])
        self.assertSequenceEqual(params, [{'name': 'st', 'type': 'Test'}, {'name': 'st', 'type': 'Test'}])

        hour = stack["properties"]["public"][2]
        params = parser._checkout_function_pointer_param(hour["type"])
        self.assertSequenceEqual(params, [{'name': 'st', 'type': 'Test *'}, {'name': 'st', 'type': 'Test *'}])

        day = stack["properties"]["public"][3]
        params = parser._checkout_function_pointer_param(day["type"])
        self.assertSequenceEqual(params, [{'name': 'st', 'type': 'Test *'}, {'name': 'st', 'type': 'Test *'}])

        month = stack["properties"]["public"][4]
        params = parser._checkout_function_pointer_param(month["type"])
        self.assertSequenceEqual(params, [{'name': 'str', 'type': 'std::string'},
                                          {'name': 'a', 'type': 'unsigned int'},
                                          {'name': 'rand_name_0', 'type': 'unsigned short'},
                                          {'name': 'c', 'type': 'Test *'}])

        year = stack["properties"]["public"][5]
        params = parser._checkout_function_pointer_param(year["type"])
        self.assertSequenceEqual(params, [{'name': 'out1', 'type': 'unsigned int * *'},
                                          {'name': 'out2', 'type': 'Test * *'}])

    def test_checkout_function_pointer_param_with_struct(self):
        header_file = """
            typedef struct {
                int32_t (*GetPortCapability)(struct AudioAdapter *adapter, const struct AudioPort *port,
                                             struct AudioPortCapability *capability);
            } AudioAdapter;
        """
        parser = HeaderParser()
        hjson = json.loads(CppHeaderParser.CppHeader(header_file, "string").toJSON())
        stack = hjson["classes"]["AudioAdapter"]
        function = stack["properties"]["public"][0]
        params = parser._checkout_function_pointer_param(function["type"])
        self.assertSequenceEqual(params, [{'name': 'adapter', 'type': 'AudioAdapter *'},
                                          {'name': 'port', 'type': 'AudioPort *'},
                                          {'name': 'capability', 'type': 'AudioPortCapability *'}])


if __name__ == "__main__":
    unittest.main()
