#include <core_init.h>
// #include <haversine.h>
// #include <json_serializer.h>

// struct CommandLineArguments {
//     f64 answer;
//     core::StrBuilder<> inFileSb;
// };

// void printUsage() {
//     printf("Usage: ./haversine [input.json] [expected answer]\n");
//     printf("Options:\n");
//     printf("  --help: Print this help message\n");
// }

// bool parserCmdArguments(i32 argc, const char** argv, CommandLineArguments& cmdArgs) {
//     core::CmdFlagParser flagParser;

//     Expect(
//         flagParser.parse(argc, argv),
//         "Failed to parse command line arguments!"
//     );

//     if (flagParser.argumentCount() < 2) {
//         fprintf(stderr, "Invalid number of arguments!\n");
//         printUsage();
//         return false;
//     }

//     bool argsOk = true;

//     flagParser.arguments([&argsOk, &cmdArgs](core::StrView arg, addr_size idx) {
//         switch (idx)
//         {
//             case 0:
//                 cmdArgs.inFileSb = core::StrBuilder<>(arg);
//                 break;

//             case 1:
//                 cmdArgs.answer = core::cptrToFloat<f64>(arg.data());
//                 break;

//             default:
//                 argsOk = false;
//                 return false;
//         }

//         return true;
//     });

//     if (!argsOk) {
//         return false;
//     }

//     return true;
// }

i32 main(i32, const char**) {
//     if (!initCore(argc, argv)) {
//         printf("%s\n", "Failed to initialize core!");
//         return -1;
//     }

//     auto serDeserFn = [](auto& sd,
//                     core::StrView& key1, i32& v1,
//                     core::StrView& key2, f64& v2,
//                     core::StrView& key3, bool& v3,
//                     core::StrView& arrKey, i32* arr, addr_size arrLen) {
//         sd.obj();
//             sd.key(key1);
//             sd.val(v1);
//             sd.obj();
//                 sd.key(key2);
//                 sd.val(v2);
//                 sd.key(key3);
//                 sd.val(v3, true);
//             sd.objEnd();
//             sd.key(arrKey);
//             sd.arr();
//                 for (addr_size i = 0; i < arrLen; ++i) {
//                     sd.val(arr[i], i == arrLen - 1);
//                 }
//             sd.arrEnd(true);
//         sd.objEnd(true);
//     };

//     // core::JSONSerializer ser;

//     // {
//     //     core::StrView key1 = core::sv("key1");
//     //     core::StrView key2 = core::sv("key2 with spaces");
//     //     core::StrView key3 = core::sv("key3");
//     //     core::StrView arrKey = core::sv("arrKey");
//     //     i32 v1 = 123;
//     //     f64 v2 = 123.456;
//     //     bool v3 = true;
//     //     i32 arr[] = { 5, 10, 20 };

//     //     serDeserFn(ser, key1, v1, key2, v2, key3, v3, arrKey, arr, 3);
//     // }

//     // printf("%s\n", "Serialized JSON:");
//     // printf("%s\n", ser.json.view().data());
//     // printf("\n");

//     CommandLineArguments cmdArgs;
//     if (!parserCmdArguments(argc, argv, cmdArgs)) {
//         return -2;
//     }

//     printf("Input file: %s\n", cmdArgs.inFileSb.view().data());
//     printf("Expected answer: %.16f\n", cmdArgs.answer);

//     core::JSONDeserializer deser;
//     deser.json = cmdArgs.inFileSb.view();

//     printf("%s\n", "Deserialized JSON:");
//     {
//         core::StrView key1, key2, key3, arrKey;
//         i32 v1;
//         f64 v2;
//         bool v3;
//         i32 arr[3];

//         serDeserFn(deser, key1, v1, key2, v2, key3, v3, arrKey, arr, 3);

//         printf("\"%.*s\" : %d\n", i32(key1.len()), key1.data(), v1);
//         printf("\"%.*s\" : %.16f\n", i32(key2.len()), key2.data(), v2);
//         printf("\"%.*s\" : %s\n", i32(key3.len()), key3.data(), v3 ? "true" : "false");
//         printf("\"%.*s\" : [%d, %d, %d]\n", i32(arrKey.len()), arrKey.data(), arr[0], arr[1], arr[2]);
//     }

    return 0;
}

