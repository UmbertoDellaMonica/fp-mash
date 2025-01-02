import 'package:flutter/material.dart';
import 'package:fp_mash/screen/steps/step_2_screen.dart';
import 'package:fp_mash/screen/steps/step_3_screen.dart';
import 'package:fp_mash/screen/steps/step_4_screen.dart';
import 'package:fp_mash/screen/steps/step_5_screen.dart';
import 'package:oktoast/oktoast.dart';

import 'screen/home/home_screen.dart';
import 'screen/steps/step_1_screen.dart';

void main() {
  runApp(
    OKToast(child:
  
  MyApp())
  );
}

class MyApp extends StatefulWidget {
  const MyApp({super.key});

  @override
  State<MyApp> createState() => _MyAppState();
}

class _MyAppState extends State<MyApp> {
  // This widget is the root of your application.
  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'fp-mash Software',
      theme: ThemeData(
        colorScheme: ColorScheme.fromSeed(seedColor: Colors.deepPurple),
        useMaterial3: true,
      ),
      home: const MyHomePage(title: 'fp-mash'),
      routes: {
        '/step1': (context) => Step1Screen(),
        '/step2': (context) => Step2Screen(),
        '/step3': (context) => Step3Screen(),
        '/step4': (context) => Step4Screen(),
        '/step5': (context) => Step5Screen()
      },
      debugShowCheckedModeBanner: false,
    );
  }
}
