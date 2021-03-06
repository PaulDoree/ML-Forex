//+------------------------------------------------------------------+
//|                                                   ANNtrading.mq4 |
//|                                                      Tan Yan Han |
//|                                                                  |
//+------------------------------------------------------------------+
#property copyright "Tan Yan Han"
#property link      ""
#property version   "1.00"
#property strict

#include <hanlib.mqh>

//External Variables
extern bool DynamicLotSize = false;
extern double EquityPercent = 10;
extern int DurationToHold = 10;

int TicketTrack[10]={0};
datetime DateTrack[10];
int pointer = 0;

datetime CurrentTimeStamp;
int SellTicket;
int BuyTicket;
int SecondsDay = 86400-5;
//+------------------------------------------------------------------+
//| Expert initialization function                                   |
//+------------------------------------------------------------------+

double weight1[5][4]={{-0.98,1.44,1.94,-1},{5.2,4.75,4.8,5.3},{-4,2.47,2.41,-3.8},{0.88,1.94,2.03,-0.0157},{-1.03,1.46,1.38,-0.74}};
double weight2[5]={5.19,-0.73,-0.72,4.83,-2.35};

int OnInit()
  {
  Print("EA Activated and Ongoing");
   CurrentTimeStamp = Time[0];
   for(int i=0; i<OrdersTotal(); i++)
   {
      OrderSelect(i,SELECT_BY_POS);
      if(OrderMagicNumber()== 0003)
      {
         TicketTrack[pointer]=OrderTicket();
         DateTrack[pointer]=OrderOpenTime()+1*SecondsDay;
         Print(" Open Orders - "+IntegerToString(TicketTrack[pointer])+", "+TimeToStr(DateTrack[pointer])+" ");
         pointer++;
      }
   }
 
   return(INIT_SUCCEEDED);
  }
//+------------------------------------------------------------------+
//| Expert deinitialization function                                 |
//+------------------------------------------------------------------+
void OnDeinit(const int reason)
  {
//---
   
  }
//+------------------------------------------------------------------+
//| Expert tick function                                             |
//+------------------------------------------------------------------+
void OnTick()
{
   bool NewBar;
   if(CurrentTimeStamp != Time[0])
   {
      CurrentTimeStamp=Time[0];
      NewBar=true;
   }
   else NewBar = false;
   if(NewBar==true)
   {
      
      for(int i=0;i<pointer;i++)
      {
         if(iTime(NULL,0,0) > DateTrack[i])  //trade context cannot have any outstanding pending orders
         {
            OrderSelect(TicketTrack[i],SELECT_BY_TICKET);
            if(OrderType() == OP_BUY)
            {
               CloseBuyOrder(Symbol(),TicketTrack[i],5);
            }
            else CloseSellOrder(Symbol(),TicketTrack[i],5);
            
            for(int j=i;j<pointer-1;j++)
            {
               TicketTrack[j]=TicketTrack[j+1];
               DateTrack[j]=DateTrack[j+1];
            }
            pointer--;
            i--;
         }
      }     
      double MA5=iMA(NULL,0,5,0,MODE_SMA,PRICE_CLOSE,0);
      double MA10=iMA(NULL,0,10,0,MODE_SMA,PRICE_CLOSE,0);
      double MA20=iMA(NULL,0,20,0,MODE_SMA,PRICE_CLOSE,0);
      double MA60=iMA(NULL,0,60,0,MODE_SMA,PRICE_CLOSE,0);
      
      double length=sqrt(MA5*MA5+MA10*MA10+MA20*MA20+MA60*MA60);
      
      MA5=MA5/length;
      MA10=MA10/length;
      MA20=MA20/length;
      MA60=MA60/length;
      
      if(NeuralNetwork(MA5,MA10,MA20,MA60))
      {
      //Buy Trade
      //double StopLoss = (iClose(NULL,0,1)-iLow(NULL,0,1))*10000;
      //double LotSize = CalcLotSize(DynamicLotSize,EquityPercent, round(StopLoss), 0.1);
      double LotSize = 0.1; //VerifyLotSize(LotSize);
      int Slippage = GetSlippage(Symbol(), 5);
      BuyTicket = OpenBuyOrder(Symbol(),LotSize,Slippage,0003);
      TicketTrack[pointer]=BuyTicket;
      DateTrack[pointer]=CurrentTimeStamp+1*SecondsDay;
      pointer++;
      
      OrderSelect(BuyTicket,SELECT_BY_TICKET);
      double OpenPrice = OrderOpenPrice();
      double BuyStopLoss = 
      BuyStopLoss = AdjustBelowStopLevel(Symbol(),BuyStopLoss);
      double BuyTakeProfit =CalcBuyTakeProfit(Symbol(), 800, OpenPrice ); //This is 8 pips tp
      bool Modified = AddStopProfit(BuyTicket, BuyStopLoss, BuyTakeProfit); 
      }
      else
      {
      //Sell Trade
      //double StopLoss = (iHigh(NULL,0,1)-iClose(NULL,0,1))*10000;
      //double LotSize = CalcLotSize(DynamicLotSize,EquityPercent, round(StopLoss), 0.1);
      double LotSize = 0.1;
      int Slippage = GetSlippage(Symbol(), 5);
      SellTicket = OpenSellOrder(Symbol(),LotSize,Slippage,0002);
      TicketTrack[pointer]=SellTicket;
      DateTrack[pointer]=CurrentTimeStamp + 1*SecondsDay;
      pointer++;
      
      OrderSelect(SellTicket,SELECT_BY_TICKET);
      double OpenPrice = OrderOpenPrice();
      double SellStopLoss = 1000;
      SellStopLoss = AdjustAboveStopLevel(Symbol(),SellStopLoss);
      double SellTakeProfit = CalcSellTakeProfit(Symbol(), 800, OpenPrice ); //This is like 8 pips take profit
      bool Modified = AddStopProfit(SellTicket, SellStopLoss, SellTakeProfit); 
      }
      
    }
  return;     
//---
   
}
//+------------------------------------------------------------------+
 
 bool NeuralNetwork(double arg5MA, double arg10MA, double arg20MA, double arg60MA)
 { 
   double input0=arg5MA*weight1[0][0]+arg10MA*weight1[1][0]+arg20MA*weight1[2][0]+arg60MA*weight1[3][0]+1.0*weight1[4][0];
   double input1=arg5MA*weight1[0][1]+arg10MA*weight1[1][1]+arg20MA*weight1[2][1]+arg60MA*weight1[3][1]+1.0*weight1[4][1];
   double input2=arg5MA*weight1[0][2]+arg10MA*weight1[1][2]+arg20MA*weight1[2][2]+arg60MA*weight1[3][2]+1.0*weight1[4][2];
   double input3=arg5MA*weight1[0][3]+arg10MA*weight1[1][3]+arg20MA*weight1[2][3]+arg60MA*weight1[3][3]+1.0*weight1[4][3];
   
   double output0=sigmoid(input0);
   double output1=sigmoid(input1);
   double output2=sigmoid(input2);
   double output3=sigmoid(input3);
   
   double HtoOutput=output0*weight2[0]+output1*weight2[1]+output2*weight2[2]+output3*weight2[3]+1.0*weight2[4];
   
   double final=sigmoid(HtoOutput);
   
   Print(" "+DoubleToStr(final)+" ");
   
   if(final>0.5)
   return true;
   else
   return false; 
   
 }
   
 double sigmoid(double argx)
 {
   return (1/(1+exp(-1.0*argx)));
 }